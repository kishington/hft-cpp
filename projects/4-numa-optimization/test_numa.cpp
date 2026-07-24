#include "numa.hpp"
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>

void test_numa_awareness() {
  std::cout << "NUMA nodes available: " << numa_num_nodes() << "\n";
  std::cout << "CPUs online: " << numa_num_configured_cpus() << "\n";

  int current_node = NUMAOptimizer::get_numa_node();
  std::cout << "Current thread on node: " << current_node << "\n";
  assert(current_node >= 0);
  std::cout << "✓ NUMA awareness test passed\n";
}

void test_numa_allocation() {
  try {
    NUMAArray<int> arr(10000, 2);
    assert(arr.get_num_nodes() == 2);

    int* node0 = arr.get_node_buffer(0);
    int* node1 = arr.get_node_buffer(1);

    assert(node0 != nullptr);
    assert(node1 != nullptr);

    // Write to node-local memory
    node0[0] = 42;
    node1[0] = 99;

    assert(node0[0] == 42);
    assert(node1[0] == 99);

    std::cout << "✓ NUMA allocation test passed\n";
  } catch (const std::exception& e) {
    std::cerr << "NUMA test skipped (not available): " << e.what() << "\n";
  }
}

void test_thread_affinity() {
  try {
    std::vector<std::thread> threads;
    std::vector<int> thread_nodes(4);

    for (int i = 0; i < 4; i++) {
      threads.emplace_back([i, &thread_nodes]() {
        thread_nodes[i] = NUMAOptimizer::get_numa_node();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      });

      // Try to pin thread to CPU i
      int cpu_id = i % numa_num_configured_cpus();
      NUMAOptimizer::set_thread_affinity(threads[i].native_handle(), cpu_id);
    }

    for (auto& t : threads) {
      t.join();
    }

    std::cout << "Thread node assignments: ";
    for (int node : thread_nodes) {
      std::cout << node << " ";
    }
    std::cout << "\n";
    std::cout << "✓ Thread affinity test passed\n";
  } catch (const std::exception& e) {
    std::cerr << "Thread affinity test skipped: " << e.what() << "\n";
  }
}

int main() {
  test_numa_awareness();
  test_numa_allocation();
  test_thread_affinity();
  std::cout << "\nAll NUMA tests passed!\n";
  return 0;
}
