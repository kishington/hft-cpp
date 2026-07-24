#pragma once

#include <atomic>
#include <cstdint>
#include <cstring>
#include <vector>

template <size_t MESSAGE_SIZE = 256, size_t POOL_SIZE = 100000>
class LockFreeMessageQueue {
private:
  struct Node {
    int64_t timestamp_ns;
    uint8_t message[MESSAGE_SIZE];
    std::atomic<Node*> next{nullptr};
  };

  // Pre-allocated node pool (no malloc/free in hot path)
  std::vector<Node> node_pool;
  alignas(64) std::atomic<Node*> free_list;

  // Head and tail for FIFO ordering
  alignas(64) std::atomic<Node*> head;
  alignas(64) std::atomic<Node*> tail;

public:
  LockFreeMessageQueue() : node_pool(POOL_SIZE) {
    // Link all nodes into free list
    for (size_t i = 0; i < POOL_SIZE - 1; i++) {
      node_pool[i].next.store(&node_pool[i + 1], std::memory_order_relaxed);
    }
    node_pool[POOL_SIZE - 1].next.store(nullptr, std::memory_order_relaxed);

    // Initialize free list and sentinel node
    free_list.store(&node_pool[0], std::memory_order_release);
    Node* sentinel = new Node();
    head.store(sentinel, std::memory_order_relaxed);
    tail.store(sentinel, std::memory_order_relaxed);
  }

  ~LockFreeMessageQueue() {
    // Clean up sentinel
    delete head.load();
  }

  // Enqueue message (lock-free)
  bool enqueue(const uint8_t* msg, size_t len, int64_t ts) {
    if (len > MESSAGE_SIZE) return false;

    // Try to allocate a node from free list
    Node* new_node = free_list.load(std::memory_order_acquire);
    if (!new_node) return false;  // No free nodes

    // CAS to remove from free list
    Node* next_free = new_node->next.load(std::memory_order_relaxed);
    if (!free_list.compare_exchange_strong(new_node, next_free,
                                            std::memory_order_release,
                                            std::memory_order_acquire)) {
      return false;  // Failed to grab node
    }

    // Initialize node
    new_node->timestamp_ns = ts;
    std::memcpy(new_node->message, msg, len);
    new_node->next.store(nullptr, std::memory_order_relaxed);

    // Link into queue
    Node* last = tail.load(std::memory_order_relaxed);
    while (!last->next.compare_exchange_weak(nullptr, new_node,
                                              std::memory_order_release,
                                              std::memory_order_relaxed)) {
      last = tail.load(std::memory_order_acquire);
    }
    tail.store(new_node, std::memory_order_release);
    return true;
  }

  // Dequeue message (lock-free)
  bool dequeue(uint8_t* msg, size_t& len, int64_t& ts) {
    Node* first = head.load(std::memory_order_acquire);
    Node* next = first->next.load(std::memory_order_acquire);

    if (!next) return false;  // Queue empty (only sentinel)

    // Copy data
    std::memcpy(msg, next->message, MESSAGE_SIZE);
    len = MESSAGE_SIZE;
    ts = next->timestamp_ns;

    // Try to advance head
    if (head.compare_exchange_strong(first, next,
                                      std::memory_order_release,
                                      std::memory_order_acquire)) {
      // Return node to free list
      Node* old_head = first;
      old_head->next.store(free_list.load(std::memory_order_relaxed),
                           std::memory_order_relaxed);
      while (!free_list.compare_exchange_weak(old_head->next, old_head,
                                               std::memory_order_release,
                                               std::memory_order_relaxed)) {}
      return true;
    }
    return false;
  }

  size_t approximate_size() const {
    // Rough estimate (not exact due to concurrency)
    return POOL_SIZE;
  }
};
