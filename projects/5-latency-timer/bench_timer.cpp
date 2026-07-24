#include "timer.hpp"
#include <iostream>
#include <cstdio>
#include <cstring>

void benchmark_operation_latencies() {
  LatencyTimer timer;
  LatencyStatistics stats;

  printf("Benchmarking individual operation latencies...\n\n");

  // 1. NOP latency
  for (int i = 0; i < 100000; i++) {
    timer.start();
    asm volatile("nop");
    timer.stop();
    stats.record(timer.latency_ns());
  }
  printf("Operation: NOP\n");
  stats.analyze();

  // 2. Memory read latency
  stats = LatencyStatistics();
  int arr[1024];
  for (int i = 0; i < 100000; i++) {
    timer.start();
    volatile int val = arr[i % 1024];
    timer.stop();
    stats.record(timer.latency_ns());
    (void)val;
  }
  printf("\nOperation: L1 Cache Read\n");
  stats.analyze();

  // 3. Function call latency
  stats = LatencyStatistics();
  auto dummy_func = [](int x) { return x + 1; };

  for (int i = 0; i < 100000; i++) {
    timer.start();
    volatile int result = dummy_func(i);
    timer.stop();
    stats.record(timer.latency_ns());
    (void)result;
  }
  printf("\nOperation: Simple Function Call\n");
  stats.analyze();

  // 4. Atomic load latency
  stats = LatencyStatistics();
  std::atomic<int> atomic_val{42};

  for (int i = 0; i < 100000; i++) {
    timer.start();
    volatile int val = atomic_val.load(std::memory_order_relaxed);
    timer.stop();
    stats.record(timer.latency_ns());
    (void)val;
  }
  printf("\nOperation: Atomic Load (memory_order_relaxed)\n");
  stats.analyze();

  // 5. CAS (Compare-and-swap) latency
  stats = LatencyStatistics();
  std::atomic<int> atomic_val2{1};

  for (int i = 0; i < 100000; i++) {
    timer.start();
    int expected = 1;
    atomic_val2.compare_exchange_weak(expected, 1,
                                      std::memory_order_relaxed,
                                      std::memory_order_relaxed);
    timer.stop();
    stats.record(timer.latency_ns());
  }
  printf("\nOperation: Compare-and-Swap (no contention)\n");
  stats.analyze();

  // 6. Memcpy latency
  stats = LatencyStatistics();
  char src[256], dst[256];
  std::memset(src, 0xAB, 256);

  for (int i = 0; i < 10000; i++) {
    timer.start();
    std::memcpy(dst, src, 256);
    timer.stop();
    stats.record(timer.latency_ns());
  }
  printf("\nOperation: Memcpy (256 bytes)\n");
  stats.analyze();
}

int main() {
  benchmark_operation_latencies();
  std::cout << "\n✓ Benchmarks complete\n";
  return 0;
}
