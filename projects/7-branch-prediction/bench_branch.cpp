#include "branch.hpp"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstdio>

uint64_t get_time_ns() {
  return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}

void benchmark_branch_prediction() {
  const size_t ARRAY_SIZE = 100000;
  uint8_t data[ARRAY_SIZE];

  std::cout << "\n=== Branch Prediction Benchmark ==="
            << "\nArray size: " << ARRAY_SIZE << " bytes\n\n";

  // Test 1: Sorted data (highly predictable)
  std::cout << "Sorted data (predictable branch):\n";
  for (size_t i = 0; i < ARRAY_SIZE; i++) {
    data[i] = i % 256;
  }

  uint64_t start = get_time_ns();
  uint64_t result1 = BranchPredictor::unpredictable_branch(data, ARRAY_SIZE);
  uint64_t end = get_time_ns();
  printf("  Unpredictable: %.2f ns/elem, result=%lu\n",
         (double)(end - start) / ARRAY_SIZE, result1);

  start = get_time_ns();
  uint64_t result2 = BranchPredictor::predictable_branch(data, ARRAY_SIZE);
  end = get_time_ns();
  printf("  Predictable:   %.2f ns/elem, result=%lu\n",
         (double)(end - start) / ARRAY_SIZE, result2);

  start = get_time_ns();
  uint64_t result3 = BranchPredictor::branchless_computation(data, ARRAY_SIZE);
  end = get_time_ns();
  printf("  Branchless:    %.2f ns/elem, result=%lu\n",
         (double)(end - start) / ARRAY_SIZE, result3);

  // Test 2: Random data (unpredictable)
  std::cout << "\nRandom data (unpredictable branch):\n";
  for (size_t i = 0; i < ARRAY_SIZE; i++) {
    data[i] = rand() % 256;
  }

  start = get_time_ns();
  result1 = BranchPredictor::unpredictable_branch(data, ARRAY_SIZE);
  end = get_time_ns();
  printf("  Unpredictable: %.2f ns/elem, result=%lu\n",
         (double)(end - start) / ARRAY_SIZE, result1);

  start = get_time_ns();
  result2 = BranchPredictor::predictable_branch(data, ARRAY_SIZE);
  end = get_time_ns();
  printf("  Predictable:   %.2f ns/elem, result=%lu\n",
         (double)(end - start) / ARRAY_SIZE, result2);

  start = get_time_ns();
  result3 = BranchPredictor::branchless_computation(data, ARRAY_SIZE);
  end = get_time_ns();
  printf("  Branchless:    %.2f ns/elem, result=%lu\n",
         (double)(end - start) / ARRAY_SIZE, result3);
}

void benchmark_lookup_table() {
  const size_t ARRAY_SIZE = 100000;
  uint8_t data[ARRAY_SIZE];

  // Random data
  for (size_t i = 0; i < ARRAY_SIZE; i++) {
    data[i] = rand() % 256;
  }

  std::cout << "\n=== Lookup Table Optimization ==="
            << "\nArray size: " << ARRAY_SIZE << " bytes\n\n";

  uint64_t start = get_time_ns();
  uint64_t result1 = BranchPredictor::branchless_computation(data, ARRAY_SIZE);
  uint64_t end = get_time_ns();
  printf("Branchless:      %.2f ns/elem, result=%lu\n",
         (double)(end - start) / ARRAY_SIZE, result1);

  LookupTableOptimizer lut;
  start = get_time_ns();
  uint64_t result2 = lut.process_with_lut(data, ARRAY_SIZE);
  end = get_time_ns();
  printf("Lookup table:    %.2f ns/elem, result=%lu\n",
         (double)(end - start) / ARRAY_SIZE, result2);
}

void benchmark_vectorization() {
  const size_t ARRAY_SIZE = 100000;
  uint8_t data[ARRAY_SIZE];

  for (size_t i = 0; i < ARRAY_SIZE; i++) {
    data[i] = rand() % 256;
  }

  std::cout << "\n=== Vectorization Impact ==="
            << "\nArray size: " << ARRAY_SIZE << " bytes\n\n";

  uint64_t start = get_time_ns();
  uint64_t result1 = BranchPredictor::branchless_computation(data, ARRAY_SIZE);
  uint64_t end = get_time_ns();
  printf("Scalar loop:     %.2f ns/elem, result=%lu\n",
         (double)(end - start) / ARRAY_SIZE, result1);

  start = get_time_ns();
  uint64_t result2 = VectorizedBranch::vectorized_computation(data, ARRAY_SIZE);
  end = get_time_ns();
  printf("Vectorized:      %.2f ns/elem, result=%lu\n",
         (double)(end - start) / ARRAY_SIZE, result2);
}

void benchmark_data_layout() {
  const size_t ARRAY_SIZE = 10000;

  std::cout << "\n=== Data Layout Impact ==="
            << "\nArray size: " << ARRAY_SIZE << " elements\n\n";

  // AoS layout
  DataLayoutOptimizer::AoS* aos_data =
      new DataLayoutOptimizer::AoS[ARRAY_SIZE];
  for (size_t i = 0; i < ARRAY_SIZE; i++) {
    aos_data[i].value = rand() % 256;
  }

  uint64_t start = get_time_ns();
  uint64_t result1 =
      DataLayoutOptimizer::aos_processing(aos_data, ARRAY_SIZE);
  uint64_t end = get_time_ns();
  printf("AoS layout:      %.2f ns/elem, result=%lu\n",
         (double)(end - start) / ARRAY_SIZE, result1);

  // SoA layout
  DataLayoutOptimizer::SoA soa_data;
  for (size_t i = 0; i < ARRAY_SIZE; i++) {
    soa_data.values.push_back(aos_data[i].value);
  }

  start = get_time_ns();
  uint64_t result2 = DataLayoutOptimizer::soa_processing(soa_data);
  end = get_time_ns();
  printf("SoA layout:      %.2f ns/elem, result=%lu\n",
         (double)(end - start) / ARRAY_SIZE, result2);

  delete[] aos_data;
}

int main() {
  std::cout << "Benchmarking branch prediction and CPU optimization...\n";
  benchmark_branch_prediction();
  benchmark_lookup_table();
  benchmark_vectorization();
  benchmark_data_layout();
  std::cout << "\n✓ Benchmarks complete\n";
  return 0;
}
