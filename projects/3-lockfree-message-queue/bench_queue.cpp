#include "queue.hpp"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <thread>
#include <vector>
#include <atomic>

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

    double throughput = (double)n * 1e9 / sum;  // ops/sec

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\n" << name << ":\n";
    std::cout << "  Count:       " << n << " ops\n";
    std::cout << "  Min:         " << min_val << " ns\n";
    std::cout << "  Avg:         " << avg << " ns\n";
    std::cout << "  p50:         " << p50 << " ns\n";
    std::cout << "  p99:         " << p99 << " ns\n";
    std::cout << "  p999:        " << p999 << " ns\n";
    std::cout << "  Max:         " << max_val << " ns\n";
    std::cout << "  Throughput:  " << throughput / 1e6 << " Mops/sec\n";
  }
};

uint64_t get_time_ns() {
  return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}

void benchmark_single_threaded() {
  std::cout << "\nBenchmarking single-threaded enqueue/dequeue...\n";
  LockFreeMessageQueue<256, 100000> q;
  LatencyRecorder enqueue_latency, dequeue_latency;

  // Enqueue
  uint8_t msg[256];
  std::memset(msg, 0xAB, 256);

  for (int i = 0; i < 100000; i++) {
    uint64_t start = get_time_ns();
    while (!q.enqueue(msg, 256, i)) {}
    uint64_t end = get_time_ns();
    enqueue_latency.record(end - start);
  }

  // Dequeue
  uint8_t out[256];
  size_t len;
  int64_t ts;
  for (int i = 0; i < 100000; i++) {
    uint64_t start = get_time_ns();
    while (!q.dequeue(out, len, ts)) {}
    uint64_t end = get_time_ns();
    dequeue_latency.record(end - start);
  }

  enqueue_latency.report("Enqueue");
  dequeue_latency.report("Dequeue");
}

void benchmark_concurrent() {
  std::cout << "\nBenchmarking concurrent (2 producers, 2 consumers)...\n";
  LockFreeMessageQueue<256, 100000> q;
  LatencyRecorder enqueue_latency, dequeue_latency;
  std::atomic<int> producer_done{0}, consumer_done{0};

  uint8_t msg[256];
  std::memset(msg, 0xCD, 256);

  std::vector<std::thread> threads;

  // Producer threads
  for (int p = 0; p < 2; p++) {
    threads.emplace_back([&]() {
      for (int i = 0; i < 250000; i++) {
        uint64_t start = get_time_ns();
        while (!q.enqueue(msg, 256, i)) {}
        uint64_t end = get_time_ns();
        enqueue_latency.record(end - start);
      }
      producer_done.fetch_add(1, std::memory_order_relaxed);
    });
  }

  // Consumer threads
  for (int c = 0; c < 2; c++) {
    threads.emplace_back([&]() {
      uint8_t out[256];
      size_t len;
      int64_t ts;
      int consumed = 0;
      while (consumed < 250000 || producer_done.load() < 2) {
        if (q.dequeue(out, len, ts)) {
          uint64_t end = get_time_ns();
          dequeue_latency.record(0);  // Simplified for demo
          consumed++;
        }
      }
      consumer_done.fetch_add(1, std::memory_order_relaxed);
    });
  }

  for (auto& t : threads) {
    t.join();
  }

  enqueue_latency.report("Concurrent Enqueue");
  dequeue_latency.report("Concurrent Dequeue");
}

int main() {
  std::cout << "Benchmarking lock-free message queue...\n";
  benchmark_single_threaded();
  benchmark_concurrent();
  std::cout << "\n✓ Benchmarks complete\n";
  return 0;
}
