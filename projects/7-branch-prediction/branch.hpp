#pragma once

#include <cstdint>
#include <algorithm>
#include <vector>
#include <cstring>

class BranchOptimizer {
public:
  // Branch prediction hints
  static inline bool likely(bool condition) {
    return __builtin_expect(condition, 1);
  }

  static inline bool unlikely(bool condition) {
    return __builtin_expect(condition, 0);
  }

  // Compiler barriers to prevent optimization reordering
  static inline void compiler_barrier() {
    asm volatile("");
  }

  // CPU memory barrier (serializing)
  static inline void memory_barrier() {
    asm volatile("mfence" ::: "memory");
  }
};

// Demonstrates branch prediction impact
class BranchPredictor {
public:
  // Worst case: unpredictable branch
  static uint64_t unpredictable_branch(const uint8_t* data, size_t n) {
    uint64_t sum = 0;
    for (size_t i = 0; i < n; i++) {
      // Random data causes mispredictions
      if (data[i] > 128) {
        sum += data[i];
      }
    }
    return sum;
  }

  // Good case: predictable branch
  static uint64_t predictable_branch(const uint8_t* data, size_t n) {
    uint64_t sum = 0;
    for (size_t i = 0; i < n; i++) {
      // Hint compiler about expected condition
      if (BranchOptimizer::likely(data[i] > 128)) {
        sum += data[i];
      }
    }
    return sum;
  }

  // Best case: branchless computation
  static uint64_t branchless_computation(const uint8_t* data, size_t n) {
    uint64_t sum = 0;
    for (size_t i = 0; i < n; i++) {
      // No branch, just conditional arithmetic
      sum += data[i] * (data[i] > 128 ? 1 : 0);
    }
    return sum;
  }

  // CMov (conditional move) - also branchless
  static uint64_t cmov_computation(const uint8_t* data, size_t n) {
    uint64_t sum = 0;
    for (size_t i = 0; i < n; i++) {
      uint64_t value = (data[i] > 128) ? data[i] : 0;
      sum += value;
    }
    return sum;
  }
};

// Lookup table (LUT) - avoids branches entirely
class LookupTableOptimizer {
private:
  uint8_t lut[256];

public:
  LookupTableOptimizer() {
    // Pre-compute results for all possible values
    for (int i = 0; i < 256; i++) {
      lut[i] = (i > 128) ? i : 0;
    }
  }

  uint64_t process_with_lut(const uint8_t* data, size_t n) const {
    uint64_t sum = 0;
    for (size_t i = 0; i < n; i++) {
      sum += lut[data[i]];
    }
    return sum;
  }
};

// SIMD-style manual vectorization (4 elements per iteration)
class VectorizedBranch {
public:
  static uint64_t vectorized_computation(const uint8_t* data, size_t n) {
    uint64_t sum = 0;
    size_t i = 0;

    // Process 4 elements at a time
    for (; i + 3 < n; i += 4) {
      sum += (data[i] > 128) ? data[i] : 0;
      sum += (data[i + 1] > 128) ? data[i + 1] : 0;
      sum += (data[i + 2] > 128) ? data[i + 2] : 0;
      sum += (data[i + 3] > 128) ? data[i + 3] : 0;
    }

    // Handle remainder
    for (; i < n; i++) {
      sum += (data[i] > 128) ? data[i] : 0;
    }

    return sum;
  }
};

// Data layout optimization
class DataLayoutOptimizer {
public:
  // AoS: Array of Structures - poor cache locality for selective fields
  struct AoS {
    uint8_t value;
    uint8_t padding[7];  // Cache line alignment
  };

  // SoA: Structure of Arrays - better for vectorization and caching
  struct SoA {
    std::vector<uint8_t> values;
    std::vector<uint8_t> flags;
  };

  static uint64_t aos_processing(const AoS* data, size_t n) {
    uint64_t sum = 0;
    for (size_t i = 0; i < n; i++) {
      if (data[i].value > 128) {
        sum += data[i].value;
      }
    }
    return sum;
  }

  static uint64_t soa_processing(const SoA& data) {
    uint64_t sum = 0;
    for (size_t i = 0; i < data.values.size(); i++) {
      if (data.values[i] > 128) {
        sum += data.values[i];
      }
    }
    return sum;
  }
};
