#pragma once
#include <iostream>
#include <cstdlib>
#include <new>

template <typename NodeType, size_t NodesPerChunk = 16>
class BPlusNodePoolAllocator {
  // Структура чанка (большого блока памяти)
  struct Chunk {
    Chunk* next_chunk;
    alignas(NodeType) char data[sizeof(NodeType) * NodesPerChunk];
  };

  // Объединение для реализации Free List без лишних затрат памяти
  union Slot {
    NodeType node;
    Slot* next_free;
  };

  Chunk* head_chunk;     // Указатель на первый выделенный чанк
  Slot* free_list_head;  // Голова списка свободных узлов
  size_t current_index;  // Индекс следующего свободного слота в текущем чанке

  // Выделение нового чанка памяти
  void allocate_new_chunk() {
    // Выделяем сырую память. Используем std::malloc, так как new может
    // вызывать конструкторы, а нам нужна просто сырая память.
    Chunk* new_chunk = static_cast<Chunk*>(std::malloc(sizeof(Chunk)));
    if (!new_chunk) throw std::bad_alloc();

    new_chunk->next_chunk = head_chunk;
    head_chunk = new_chunk;
    current_index = 0;
  }

public:
  BPlusNodePoolAllocator()
      : head_chunk(nullptr),
        free_list_head(nullptr),
        current_index(NodesPerChunk) {}

  ~BPlusNodePoolAllocator() {
    // Освобождение всех чанков
    Chunk* current = head_chunk;
    while (current) {
      Chunk* next = current->next_chunk;
      std::free(current);
      current = next;
    }
  }

  // Запрет копирования
  BPlusNodePoolAllocator<NodeType, NodesPerChunk>(
      const BPlusNodePoolAllocator<NodeType, NodesPerChunk>&) = delete;
  BPlusNodePoolAllocator<NodeType, NodesPerChunk>& operator=(
      const BPlusNodePoolAllocator<NodeType, NodesPerChunk>&) = delete;

  // Выделение памяти под один узел
  NodeType* allocate() {
    // 1. Сначала проверяем Free List (освобождённые ранее узлы)
    if (free_list_head) {
      Slot* free_slot = free_list_head;
      free_list_head = free_list_head->next_free;
      return &(free_slot->node);
    }

    // 2. Если Free List пуст, берем из текущего чанка
    if (current_index >= NodesPerChunk) {
      allocate_new_chunk();
    }

    Slot* slots = reinterpret_cast<Slot*>(head_chunk->data);
    NodeType* allocated_node = &(slots[current_index].node);
    current_index++;

    return allocated_node;
  }

  // Освобождение памяти узла (возврат в Free List)
  void deallocate(NodeType* node_ptr) {
    if (!node_ptr) return;

    // Преобразуем указатель на узел обратно в указатель на слот
    Slot* slot_ptr = reinterpret_cast<Slot*>(node_ptr);

    // Вставляем в начало Free List
    slot_ptr->next_free = free_list_head;
    free_list_head = slot_ptr;
  }
};
