#include "timer.hpp"
#include <cassert>
#include <iostream>
#include <cstdio>

void test_timer_basic() {
  LatencyTimer timer;

  timer.start();
  // Busy loop for ~1000 cycles
  for (volatile int i = 0; i < 100; i++) {
  }
  timer.stop();

  uint64_t latency_ns = timer.latency_ns();
  assert(latency_ns > 0);
  assert(latency_ns < 100000);  // Should be < 100us

  printf("Simple loop latency: %lu ns (%lu cycles)\n", latency_ns,
         timer.latency_cycles());
  std::cout << "✓ Timer basic test passed\n";
}

void test_timer_precision() {
  LatencyTimer timer;
  LatencyStatistics stats;

  // Measure precision of multiple short operations
  for (int i = 0; i < 10000; i++) {
    timer.start();
    asm volatile("nop");
    timer.stop();
    stats.record(timer.latency_ns());
  }

  uint64_t p99 = stats.get_p99();
  printf("NOP operation p99: %lu ns\n", p99);
  assert(p99 < 1000);  // Single NOP should be < 1us even at p99

  std::cout << "✓ Timer precision test passed\n";
}

void test_timer_calibration() {
  LatencyTimer timer;
  double freq_ghz = timer.get_tsc_freq_ghz();
  double ns_per_cycle = timer.get_ns_per_cycle();

  printf("TSC calibration:\n");
  printf("  Frequency: %.2f GHz\n", freq_ghz);
  printf("  NS/cycle: %.4f\n", ns_per_cycle);

  assert(freq_ghz > 1.0 && freq_ghz < 10.0);  // Reasonable range

  std::cout << "✓ Timer calibration test passed\n";
}

void test_latency_statistics() {
  LatencyTimer timer;
  LatencyStatistics stats;

  // Simulate some operations with varying latency
  for (int i = 0; i < 100000; i++) {
    timer.start();

    // Simulate varying amounts of work
    if (i % 1000 == 0) {
      for (volatile int j = 0; j < 1000; j++) {
      }
    } else {
      asm volatile("nop");
    }

    timer.stop();
    stats.record(timer.latency_ns());
  }

  stats.analyze();
  std::cout << "✓ Latency statistics test passed\n";
}

int main() {
  test_timer_basic();
  test_timer_precision();
  test_timer_calibration();
  test_latency_statistics();
  std::cout << "\nAll timer tests passed!\n";
  return 0;
}
