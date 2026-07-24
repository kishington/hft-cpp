#include "orderbook.hpp"
#include <chrono>
#include <iostream>
#include <vector>
#include <iomanip>

class LatencyRecorder {
private:
  std::vector<uint64_t> samples;
  static constexpr int MAX_SAMPLES = 1000000;

public:
  void record(uint64_t ns) {
    if (samples.size() < MAX_SAMPLES) {
      samples.push_back(ns);
    }
  }

  void report(const std::string& name) {
    if (samples.empty()) return;

    std::sort(samples.begin(), samples.end());
    int n = samples.size();

    uint64_t min_val = samples[0];
    uint64_t max_val = samples[n - 1];
    uint64_t p50 = samples[n / 2];
    uint64_t p99 = samples[n * 99 / 100];
    uint64_t p999 = samples[n * 999 / 1000];

    uint64_t sum = 0;
    for (auto s : samples) sum += s;
    uint64_t avg = sum / n;

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\n" << name << ":\n";
    std::cout << "  Count:  " << n << " ops\n";
    std::cout << "  Min:    " << min_val << " ns\n";
    std::cout << "  Avg:    " << avg << " ns\n";
    std::cout << "  p50:    " << p50 << " ns\n";
    std::cout << "  p99:    " << p99 << " ns\n";
    std::cout << "  p999:   " << p999 << " ns\n";
    std::cout << "  Max:    " << max_val << " ns\n";
  }
};

uint64_t get_time_ns() {
  return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}

int main() {
  CacheOptimizedOrderBook book;
  LatencyRecorder add_latency, cancel_latency, match_latency;

  // Benchmark: Add orders
  std::cout << "Benchmarking cache-optimized order book...\n";
  int order_id = 0;
  for (int i = 0; i < 100000; i++) {
    int price = 10050 + (i % 50);
    int side_val = i % 2;
    Side side = (side_val == 0) ? Side::BUY : Side::SELL;

    uint64_t start = get_time_ns();
    book.add_order(order_id++, price, 100, side);
    uint64_t end = get_time_ns();
    add_latency.record(end - start);
  }

  // Benchmark: Cancel orders
  for (int i = 0; i < 50000; i++) {
    int cancel_id = i * 2;
    uint64_t start = get_time_ns();
    book.cancel_order(cancel_id);
    uint64_t end = get_time_ns();
    cancel_latency.record(end - start);
  }

  // Benchmark: Market order matching
  for (int i = 0; i < 10000; i++) {
    Side side = (i % 2 == 0) ? Side::BUY : Side::SELL;
    uint64_t start = get_time_ns();
    book.match_market_order(1000, side);
    uint64_t end = get_time_ns();
    match_latency.record(end - start);
  }

  add_latency.report("Add Order");
  cancel_latency.report("Cancel Order");
  match_latency.report("Market Order Match");

  std::cout << "\n✓ Benchmarks complete\n";
  return 0;
}
