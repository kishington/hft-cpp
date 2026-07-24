#pragma once

#include <cstdint>
#include <cstring>
#include <algorithm>
#include <vector>

class CacheOptimizer {
public:
  // L1 cache typical size: 32 KB per core
  static constexpr size_t L1_CACHE_SIZE = 32 * 1024;
  // L1 cache line size: 64 bytes
  static constexpr size_t CACHE_LINE_SIZE = 64;
  // L2 cache typical size: 256 KB per core
  static constexpr size_t L2_CACHE_SIZE = 256 * 1024;
  // L3 cache typical size: 8 MB per core (shared)
  static constexpr size_t L3_CACHE_SIZE = 8 * 1024 * 1024;

  // Prefetch hint distances
  static constexpr int PREFETCH_DISTANCE = 8;
};

template <typename T>
class CacheAlignedArray {
private:
  std::vector<T> data;
  size_t size;

public:
  CacheAlignedArray(size_t n) : size(n) {
    // Allocate with cache line alignment
    size_t aligned_size =
        ((n * sizeof(T) + CacheOptimizer::CACHE_LINE_SIZE - 1) /
         CacheOptimizer::CACHE_LINE_SIZE) *
        CacheOptimizer::CACHE_LINE_SIZE;
    data.resize(aligned_size / sizeof(T));
  }

  T& operator[](size_t i) { return data[i]; }
  const T& operator[](size_t i) const { return data[i]; }
  size_t get_size() const { return size; }
  T* data_ptr() { return data.data(); }
};

// Sequential access pattern (good cache locality)
template <typename T>
class SequentialAccessor {
public:
  static void access_pattern(const T* data, size_t n) {
    volatile T sum = 0;
    for (size_t i = 0; i < n; i++) {
      sum += data[i];
    }
  }
};

// Random access pattern (poor cache locality)
template <typename T>
class RandomAccessor {
private:
  std::vector<size_t> indices;

public:
  RandomAccessor(size_t n) : indices(n) {
    for (size_t i = 0; i < n; i++) {
      indices[i] = i;
    }
    // Randomize indices
    std::random_shuffle(indices.begin(), indices.end());
  }

  void access_pattern(const T* data, size_t n) {
    volatile T sum = 0;
    for (size_t i = 0; i < n; i++) {
      sum += data[indices[i] % n];
    }
  }
};

// Strided access pattern (poor cache locality with large stride)
template <typename T>
class StridedAccessor {
private:
  size_t stride;

public:
  StridedAccessor(size_t s) : stride(s) {}

  void access_pattern(const T* data, size_t n) {
    volatile T sum = 0;
    for (size_t i = 0; i < n; i += stride) {
      sum += data[i];
    }
  }
};

// Prefetch optimization
class PrefetchHelper {
public:
  static inline void prefetch_for_read(const void* ptr) {
    __builtin_prefetch(ptr, 0, 3);  // Prefetch for read, high locality
  }

  static inline void prefetch_for_write(void* ptr) {
    __builtin_prefetch(ptr, 1, 3);  // Prefetch for write
  }
};

// Demonstrates software prefetching
template <typename T>
class PrefetchedAccess {
public:
  static void access_with_prefetch(const T* data, size_t n) {
    volatile T sum = 0;
    const int PREFETCH_DISTANCE = 16;

    // Prefetch ahead
    for (size_t i = 0; i < n; i++) {
      if (i + PREFETCH_DISTANCE < n) {
        PrefetchHelper::prefetch_for_read(
            (const void*)(data + i + PREFETCH_DISTANCE));
      }
      sum += data[i];
    }
  }
};
