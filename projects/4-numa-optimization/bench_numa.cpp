#include "numa.hpp"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <thread>
#include <vector>
#include <atomic>
#include <algorithm>

class LatencyRecorder {
private:
  std::vector<uint64_t> samples;
  static constexpr int MAX_SAMPLES = 100000;

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

    uint64_t sum = 0;
    for (auto s : samples) sum += s;
    uint64_t avg = sum / n;

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\n" << name << ":\n";
    std::cout << "  p50:    " << p50 << " ns\n";
    std::cout << "  p99:    " << p99 << " ns\n";
    std::cout << "  Avg:    " << avg << " ns\n";
  }
};

uint64_t get_time_ns() {
  return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}

void benchmark_local_vs_remote() {
  try {
    std::cout << "Benchmarking local vs remote memory access...\n";
    int num_nodes = numa_num_nodes();

    if (num_nodes < 2) {
      std::cout << "Need at least 2 NUMA nodes for this benchmark\n";
      return;
    }

    NUMAArray<uint64_t> arr(100000, num_nodes);
    LatencyRecorder local_latency, remote_latency;

    // Benchmark local access
    std::thread local_thread([&]() {
      int node = NUMAOptimizer::get_numa_node();
      uint64_t* local_buf = arr.get_node_buffer(node);

      for (int i = 0; i < 50000; i++) {
        uint64_t start = get_time_ns();
        volatile uint64_t val = local_buf[i % arr.get_node_size(node)];
        uint64_t end = get_time_ns();
        local_latency.record(end - start);
        (void)val;  // Prevent optimization
      }
    });

    // Benchmark remote access
    std::thread remote_thread([&]() {
      int node = NUMAOptimizer::get_numa_node();
      int remote_node = (node + 1) % num_nodes;
      uint64_t* remote_buf = arr.get_node_buffer(remote_node);

      for (int i = 0; i < 50000; i++) {
        uint64_t start = get_time_ns();
        volatile uint64_t val = remote_buf[i % arr.get_node_size(remote_node)];
        uint64_t end = get_time_ns();
        remote_latency.record(end - start);
        (void)val;
      }
    });

    int cpu0 = 0, cpu1 = numa_num_configured_cpus() / 2;
    NUMAOptimizer::set_thread_affinity(local_thread.native_handle(), cpu0);
    NUMAOptimizer::set_thread_affinity(remote_thread.native_handle(), cpu1);

    local_thread.join();
    remote_thread.join();

    local_latency.report("Local Memory Access");
    remote_latency.report("Remote Memory Access");
  } catch (const std::exception& e) {
    std::cerr << "Benchmark skipped: " << e.what() << "\n";
  }
}

int main() {
  std::cout << "Benchmarking NUMA optimization...\n";
  benchmark_local_vs_remote();
  std::cout << "\n✓ Benchmarks complete\n";
  return 0;
}
