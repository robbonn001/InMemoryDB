#pragma once
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include "BPlusNodePoolAllocator.h"

struct Node {
  static constexpr uint64_t N = 16;
  bool is_leaf;
  uint16_t num_keys;
};

template <typename T>
class InternalBPlusNode : private Node {
  T keys[N];
  void* children[N + 1];

 public:
  InternalBPlusNode() : num_keys(0), is_leaf(false) {}
  ~InternalBPlusNode() = default;

  // Бинарный поиск через std::lower_bound
  size_t find_child_index(const T& key) const {
    auto it = std::lower_bound(keys, keys + num_keys, key);
    return static_cast<size_t>(it - keys);
  }
  bool insert_key_and_child(const T& key, void* child) {
    children[find_child_index(key)] = child;
    num_keys++;
  }
  bool is_full() const noexcept {
      return N==num_keys;
  }
};

template <typename T>
class LeafBPlusNode : private Node {
  T keys[N];
  uint64_t row_ids[N];
  LeafBPlusNode<T>* prev;
  LeafBPlusNode<T>* next;

 public:
  LeafBPlusNode() : num_keys(0), is_leaf(true), prev(nullptr), next(nullptr) {}
  ~LeafBPlusNode() {
    if (pvev) {
      prev->next = next;
    }
  }

  int  find_key_index(const T& key);
  bool insert(const T& key, uint64_t row_id);
  bool update(const T& key, uint64_t new_row_id);
  bool remove(const T& key);
  bool is_full();
  bool is_underflow();
};

template <typename T>
class FastBPlusTree {
  void* root;
  BPlusNodePoolAllocator<InternalBPlusNode<T>> internal_pool;
  BPlusNodePoolAllocator<LeafBPlusNode<T>> leaf_pool;

  // Рекурсивный метод очистки
  void destroy_subtree(void* node);
  void* find_leaf(const T& key);
  void split_leaf(LeafBPlusNode* leaf, const T& key, uint64_t row_id);
  void split_internal(InternalBPlusNode* node, void* new_child, const T& key);
  void insert_into_parent(void* left_node, const T& key, void* right_node);

 public:
  FastBPlusTree() : root(nullptr) {}
  ~FastBPlusTree() { destroy_subtree(root);}

  void insert(const T& key, uint64_t row_id);
  uint64_t* search(const T& key);
  void update(const T& key, uint64_t new_row_id);
  void remove(const T& key);
  std::vector<uint64_t> range_search(const T& start, const T& end);
};

template <typename T>
void FastBPlusTree<T>::destroy_subtree(void* node) {
  if (!node) return;

  // Проверяем тип узла через первый байт (is_leaf)
  bool is_leaf_ptr = static_cast<Node*>(node)->is_leaf;

  if (is_leaf_ptr) {
    // Это листовой узел
    LeafBPlusNode<T>* leaf = static_cast<LeafBPlusNode<T>*>(node);
    leaf->~LeafBPlusNode();      // Вызов деструктора
    leaf_pool.deallocate(leaf);  // Возврат памяти в аллокатор
  } else {
    // Это внутренний узел
    InternalBPlusNode<T>* internal = static_cast<InternalBPlusNode<T>*>(node);

    // Рекурсивно уничтожаем всех детей
    for (size_t i = 0; i < internal->num_keys; ++i) {
      destroy_node(internal->children[i]);
    }

    internal->~InternalBPlusNode();      // Вызов деструктора
    internal_pool.deallocate(internal);  // Возврат памяти в аллокатор
  }
}

template <typename T>
void FastBPlusTree<T>::insert_into_parent(void* parent_node, const T& key,
                                                 void* child_node) {
    parent_node      
}
