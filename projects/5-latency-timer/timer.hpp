#pragma once

#include <cstdint>
#include <chrono>
#include <vector>
#include <algorithm>

class LatencyTimer {
private:
  uint64_t start_tsc;
  uint64_t end_tsc;
  double tsc_to_ns;

  static inline uint64_t rdtsc() {
    uint32_t lo, hi;
    asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
  }

  static inline uint64_t rdtscp() {
    uint32_t lo, hi, aux;
    asm volatile("rdtscp" : "=a"(lo), "=d"(hi), "=c"(aux));
    return ((uint64_t)hi << 32) | lo;
  }

public:
  LatencyTimer() : start_tsc(0), end_tsc(0) {
    // Calibrate TSC frequency
    auto clock_start = std::chrono::high_resolution_clock::now();
    uint64_t tsc_start = rdtsc();

    // Spin for 100ms
    while (std::chrono::high_resolution_clock::now() - clock_start <
           std::chrono::milliseconds(100)) {
    }

    uint64_t tsc_end = rdtsc();
    auto clock_end = std::chrono::high_resolution_clock::now();

    uint64_t tsc_diff = tsc_end - tsc_start;
    uint64_t ns_diff =
        std::chrono::duration_cast<std::chrono::nanoseconds>(clock_end -
                                                             clock_start)
            .count();

    tsc_to_ns = (double)ns_diff / tsc_diff;
  }

  // Start measurement
  inline void start() { start_tsc = rdtscp(); }

  // End measurement
  inline void stop() { end_tsc = rdtscp(); }

  // Get latency in nanoseconds
  inline uint64_t latency_ns() const {
    return (uint64_t)((end_tsc - start_tsc) * tsc_to_ns);
  }

  // Get latency in microseconds
  inline double latency_us() const {
    return ((end_tsc - start_tsc) * tsc_to_ns) / 1000.0;
  }

  // Get TSC difference (raw cycles)
  inline uint64_t latency_cycles() const { return end_tsc - start_tsc; }

  // Calibration: get TSC frequency in GHz
  double get_tsc_freq_ghz() const { return 1.0 / tsc_to_ns; }

  // Calibration: get ns per TSC cycle
  double get_ns_per_cycle() const { return tsc_to_ns; }
};

class LatencyStatistics {
private:
  std::vector<uint64_t> latencies_ns;
  static constexpr int MAX_SAMPLES = 1000000;

public:
  void record(uint64_t ns) {
    if (latencies_ns.size() < MAX_SAMPLES) {
      latencies_ns.push_back(ns);
    }
  }

  void analyze() {
    if (latencies_ns.empty()) return;

    std::sort(latencies_ns.begin(), latencies_ns.end());
    int n = latencies_ns.size();

    uint64_t min_val = latencies_ns[0];
    uint64_t max_val = latencies_ns[n - 1];
    uint64_t p50 = latencies_ns[n / 2];
    uint64_t p75 = latencies_ns[n * 3 / 4];
    uint64_t p90 = latencies_ns[n * 9 / 10];
    uint64_t p99 = latencies_ns[n * 99 / 100];
    uint64_t p999 = latencies_ns[n * 999 / 1000];
    uint64_t p9999 = latencies_ns[n * 9999 / 10000];

    uint64_t sum = 0;
    for (auto l : latencies_ns) sum += l;
    uint64_t avg = sum / n;

    printf("Latency Statistics (%d samples):\n", n);
    printf("  Min:     %8lu ns\n", min_val);
    printf("  Avg:     %8lu ns\n", avg);
    printf("  p50:     %8lu ns\n", p50);
    printf("  p75:     %8lu ns\n", p75);
    printf("  p90:     %8lu ns\n", p90);
    printf("  p99:     %8lu ns\n", p99);
    printf("  p999:    %8lu ns\n", p999);
    printf("  p9999:   %8lu ns\n", p9999);
    printf("  Max:     %8lu ns\n", max_val);
  }

  uint64_t get_p99() const {
    if (latencies_ns.empty()) return 0;
    int n = latencies_ns.size();
    return latencies_ns[n * 99 / 100];
  }
};
