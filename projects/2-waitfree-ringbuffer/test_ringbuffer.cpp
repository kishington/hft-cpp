#include "ringbuffer.hpp"
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

void test_spsc_basic() {
  WaitFreeRingBuffer<int, 256> rb;

  assert(rb.push_spsc(42));
  assert(rb.push_spsc(99));

  int val1, val2;
  assert(rb.pop_spsc(val1));
  assert(rb.pop_spsc(val2));

  assert(val1 == 42);
  assert(val2 == 99);

  int empty_val;
  assert(!rb.pop_spsc(empty_val));

  std::cout << "✓ SPSC basic test passed\n";
}

void test_spsc_full() {
  WaitFreeRingBuffer<int, 16> rb;

  // Fill completely
  for (int i = 0; i < 16; i++) {
    assert(rb.push_spsc(i));
  }

  // Should fail on 17th
  assert(!rb.push_spsc(999));

  // Pop one
  int val;
  assert(rb.pop_spsc(val));
  assert(val == 0);

  // Now should be able to push
  assert(rb.push_spsc(999));

  std::cout << "✓ SPSC full buffer test passed\n";
}

void test_mpmc_concurrent() {
  WaitFreeRingBuffer<int, 1024> rb;
  std::vector<std::thread> threads;

  // Producer threads
  for (int t = 0; t < 2; t++) {
    threads.emplace_back([&, t]() {
      for (int i = 0; i < 5000; i++) {
        while (!rb.push_mpmc(t * 10000 + i)) {
          // Retry
        }
      }
    });
  }

  // Consumer threads
  int consumed = 0;
  for (int t = 0; t < 2; t++) {
    threads.emplace_back([&]() {
      int val;
      while (consumed < 10000) {
        if (rb.pop_mpmc(val)) {
          consumed++;
        }
      }
    });
  }

  for (auto& th : threads) {
    th.join();
  }

  assert(consumed == 10000);
  std::cout << "✓ MPMC concurrent test passed\n";
}

void test_wraparound() {
  WaitFreeRingBuffer<int, 8> rb;

  // Push and pop multiple times to test wraparound
  for (int cycle = 0; cycle < 5; cycle++) {
    for (int i = 0; i < 8; i++) {
      assert(rb.push_spsc(i + cycle * 100));
    }

    for (int i = 0; i < 8; i++) {
      int val;
      assert(rb.pop_spsc(val));
      assert(val == i + cycle * 100);
    }
  }

  std::cout << "✓ Wraparound test passed\n";
}

int main() {
  test_spsc_basic();
  test_spsc_full();
  test_mpmc_concurrent();
  test_wraparound();
  std::cout << "\nAll ringbuffer tests passed!\n";
  return 0;
}
