#include "queue.hpp"
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

void test_enqueue_dequeue() {
  LockFreeMessageQueue<64, 1000> q;

  uint8_t msg1[64] = "Hello";
  assert(q.enqueue(msg1, 64, 1000));

  uint8_t msg2[64] = "World";
  assert(q.enqueue(msg2, 64, 2000));

  uint8_t out[64];
  size_t len;
  int64_t ts;

  assert(q.dequeue(out, len, ts));
  assert(std::string((char*)out) == std::string("Hello"));
  assert(ts == 1000);

  assert(q.dequeue(out, len, ts));
  assert(std::string((char*)out) == std::string("World"));
  assert(ts == 2000);

  assert(!q.dequeue(out, len, ts));  // Empty

  std::cout << "✓ Enqueue/dequeue test passed\n";
}

void test_fifo_order() {
  LockFreeMessageQueue<32, 1000> q;

  for (int i = 0; i < 100; i++) {
    uint8_t msg[32];
    std::memset(msg, i % 256, 32);
    assert(q.enqueue(msg, 32, i));
  }

  for (int i = 0; i < 100; i++) {
    uint8_t out[32];
    size_t len;
    int64_t ts;
    assert(q.dequeue(out, len, ts));
    assert(ts == i);
    assert(out[0] == (i % 256));
  }

  std::cout << "✓ FIFO order test passed\n";
}

void test_concurrent_enqueue_dequeue() {
  LockFreeMessageQueue<128, 10000> q;
  std::vector<std::thread> threads;
  std::atomic<int> enqueued{0}, dequeued{0};

  // Producer threads
  for (int p = 0; p < 2; p++) {
    threads.emplace_back([&, p]() {
      for (int i = 0; i < 2500; i++) {
        uint8_t msg[128];
        std::memset(msg, p * 10 + i, 128);
        while (!q.enqueue(msg, 128, p * 2500 + i)) {
          // Retry
        }
        enqueued.fetch_add(1, std::memory_order_relaxed);
      }
    });
  }

  // Consumer threads
  for (int c = 0; c < 2; c++) {
    threads.emplace_back([&]() {
      uint8_t out[128];
      size_t len;
      int64_t ts;
      while (dequeued.load(std::memory_order_relaxed) < 5000) {
        if (q.dequeue(out, len, ts)) {
          dequeued.fetch_add(1, std::memory_order_relaxed);
        }
      }
    });
  }

  for (auto& t : threads) {
    t.join();
  }

  assert(enqueued.load() == 5000);
  assert(dequeued.load() == 5000);
  std::cout << "✓ Concurrent test passed\n";
}

int main() {
  test_enqueue_dequeue();
  test_fifo_order();
  test_concurrent_enqueue_dequeue();
  std::cout << "\nAll queue tests passed!\n";
  return 0;
}
