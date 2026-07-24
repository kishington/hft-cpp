#include "branch.hpp"
#include <cassert>
#include <iostream>
#include <cstdlib>
#include <cstring>

void test_branch_prediction() {
  std::cout << "Testing branch prediction techniques...\n";

  // Create test data
  uint8_t data[1000];
  for (int i = 0; i < 1000; i++) {
    data[i] = rand() % 256;
  }

  // Test unpredictable branch
  uint64_t result1 = BranchPredictor::unpredictable_branch(data, 1000);
  assert(result1 > 0);

  // Test predictable branch
  uint64_t result2 = BranchPredictor::predictable_branch(data, 1000);
  assert(result2 == result1);

  // Test branchless computation
  uint64_t result3 = BranchPredictor::branchless_computation(data, 1000);
  assert(result3 == result1);

  // Test CMov computation
  uint64_t result4 = BranchPredictor::cmov_computation(data, 1000);
  assert(result4 == result1);

  std::cout << "✓ All branch variations produce same result\n";
}

void test_lookup_table() {
  uint8_t data[1000];
  for (int i = 0; i < 1000; i++) {
    data[i] = rand() % 256;
  }

  LookupTableOptimizer lut;
  uint64_t result = lut.process_with_lut(data, 1000);

  uint64_t expected = BranchPredictor::branchless_computation(data, 1000);
  assert(result == expected);

  std::cout << "✓ Lookup table optimization test passed\n";
}

void test_vectorized() {
  uint8_t data[1000];
  for (int i = 0; i < 1000; i++) {
    data[i] = rand() % 256;
  }

  uint64_t result1 = VectorizedBranch::vectorized_computation(data, 1000);
  uint64_t result2 = BranchPredictor::branchless_computation(data, 1000);
  assert(result1 == result2);

  std::cout << "✓ Vectorized computation test passed\n";
}

void test_data_layout() {
  // Test AoS
  DataLayoutOptimizer::AoS aos_data[100];
  for (int i = 0; i < 100; i++) {
    aos_data[i].value = rand() % 256;
  }
  uint64_t aos_result =
      DataLayoutOptimizer::aos_processing(aos_data, 100);

  // Test SoA
  DataLayoutOptimizer::SoA soa_data;
  for (int i = 0; i < 100; i++) {
    soa_data.values.push_back(aos_data[i].value);
  }
  uint64_t soa_result = DataLayoutOptimizer::soa_processing(soa_data);

  assert(aos_result == soa_result);

  std::cout << "✓ Data layout test passed\n";
}

int main() {
  test_branch_prediction();
  test_lookup_table();
  test_vectorized();
  test_data_layout();
  std::cout << "\nAll branch prediction tests passed!\n";
  return 0;
}
