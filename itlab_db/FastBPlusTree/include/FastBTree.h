#pragma once
#include <iostream>

template <typename T>
class InternalBPlusNode {
  static constexpr size_t size_t N = 16;
    constexpr bool is_leaf = false;
    uint16_t num_keys;
    T keys[N];
    void* children[N+1];
    public:
    InternalBPlusNode() : num_keys(0);
    ~InternalBPlusNode() {
      for (size_t i = 0; i < num_keys; ++i) {
        deallocate(children[i]);
      }
    }    
    
    void deallocate(void* child) {
      if (child) {
        bool* is_child_leaf = reinterpret_cast<bool*>(child);
        if (*is_child_leaf) {
          static_cast<LeafNode<T>*>(child)->~LeafNode();
        } else {
          static_cast<InternalNode<T>*>(child)->~InternalNode();
        }
        child = nullptr;
      }
    }
};

template <typename T>
class LeafBPlusNode {
    static constexpr size_t size_t N = 16;
    bool is_leaf;
  uint16_t num_keys;
  T keys[N];
  uint64_t row_ids[N];
  LeafNode<T>* prev;
  LeafNode<T>* next;
  public:
  LeafBPlusNode();
  ~LeafBPlusNode() {
    prev->next = next;
  }
};

template<typename T>
class FastBPlusTree {
    void* root;
    
};

