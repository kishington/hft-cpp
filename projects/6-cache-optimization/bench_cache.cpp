#include "cache.hpp"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <cstdio>

uint64_t get_time_ns() {
  return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}

volatile int prevent_optimization = 0;

void benchmark_access_patterns() {
  const size_t ARRAY_SIZE = 1024 * 1024;  // 1M elements
  CacheAlignedArray<int> arr(ARRAY_SIZE);

  // Initialize array
  for (size_t i = 0; i < ARRAY_SIZE; i++) {
    arr[i] = i % 256;
  }

  std::cout << "\n=== Access Pattern Benchmarks ==="
            << "\nArray size: " << ARRAY_SIZE << " elements (" 
            << (ARRAY_SIZE * sizeof(int)) / (1024 * 1024) << " MB)\n\n";

  // 1. Sequential access (cache-friendly)
  {
    uint64_t start = get_time_ns();
    SequentialAccessor<int>::access_pattern(arr.data_ptr(), ARRAY_SIZE);
    uint64_t end = get_time_ns();
    double time_ns = end - start;
    double throughput = (ARRAY_SIZE * sizeof(int)) / (time_ns / 1e9) / 1e9;
    printf("Sequential access:     %10.2f ns/elem, %.2f GB/s\n",
           time_ns / ARRAY_SIZE, throughput);
  }

  // 2. Strided access - cache line (64 bytes = 16 ints)
  {
    StridedAccessor<int> accessor(1);
    uint64_t start = get_time_ns();
    accessor.access_pattern(arr.data_ptr(), ARRAY_SIZE);
    uint64_t end = get_time_ns();
    double time_ns = end - start;
    double throughput = (ARRAY_SIZE * sizeof(int)) / (time_ns / 1e9) / 1e9;
    printf("Strided (stride=1):    %10.2f ns/elem, %.2f GB/s\n",
           time_ns / ARRAY_SIZE, throughput);
  }

  // 3. Strided access - L1 spill (256 bytes = 64 ints)
  {
    StridedAccessor<int> accessor(64);
    uint64_t start = get_time_ns();
    accessor.access_pattern(arr.data_ptr(), ARRAY_SIZE);
    uint64_t end = get_time_ns();
    double time_ns = end - start;
    double throughput = (ARRAY_SIZE * sizeof(int)) / (time_ns / 1e9) / 1e9;
    printf("Strided (stride=64):   %10.2f ns/elem, %.2f GB/s\n",
           time_ns / ARRAY_SIZE, throughput);
  }

  // 4. Strided access - L2 spill (4KB = 1024 ints)
  {
    StridedAccessor<int> accessor(1024);
    uint64_t start = get_time_ns();
    accessor.access_pattern(arr.data_ptr(), ARRAY_SIZE);
    uint64_t end = get_time_ns();
    double time_ns = end - start;
    double throughput = (ARRAY_SIZE * sizeof(int)) / (time_ns / 1e9) / 1e9;
    printf("Strided (stride=1024): %10.2f ns/elem, %.2f GB/s\n",
           time_ns / ARRAY_SIZE, throughput);
  }
}

void benchmark_prefetch_optimization() {
  const size_t ARRAY_SIZE = 1024 * 1024;
  CacheAlignedArray<int> arr(ARRAY_SIZE);

  for (size_t i = 0; i < ARRAY_SIZE; i++) {
    arr[i] = i % 256;
  }

  std::cout << "\n=== Prefetch Optimization Benchmark ==="
            << "\nArray size: " << ARRAY_SIZE << " elements\n\n";

  // Without prefetch
  {
    uint64_t start = get_time_ns();
    SequentialAccessor<int>::access_pattern(arr.data_ptr(), ARRAY_SIZE);
    uint64_t end = get_time_ns();
    double time_ns = end - start;
    printf("Without prefetch:      %10.2f ns/elem\n", time_ns / ARRAY_SIZE);
  }

  // With prefetch
  {
    uint64_t start = get_time_ns();
    PrefetchedAccess<int>::access_with_prefetch(arr.data_ptr(), ARRAY_SIZE);
    uint64_t end = get_time_ns();
    double time_ns = end - start;
    printf("With prefetch:         %10.2f ns/elem\n", time_ns / ARRAY_SIZE);
  }
}

void benchmark_cache_sizes() {
  std::cout << "\n=== Cache Hierarchy Performance ==="
            << "\nAccessing arrays of increasing sizes...\n\n";

  size_t sizes[] = {1024,           // 4 KB (fits in L1)
                    32 * 1024,      // 128 KB (L1 cache)
                    256 * 1024,     // 1 MB (L2 cache)
                    8 * 1024 * 1024 // 32 MB (L3 cache)
  };
  const char* names[] = {"L1", "L2", "L3", "RAM"};

  for (size_t i = 0; i < 4; i++) {
    CacheAlignedArray<int> arr(sizes[i]);
    for (size_t j = 0; j < sizes[i]; j++) {
      arr[j] = j % 256;
    }

    uint64_t start = get_time_ns();
    for (int iter = 0; iter < 1000; iter++) {
      SequentialAccessor<int>::access_pattern(arr.data_ptr(), sizes[i]);
    }
    uint64_t end = get_time_ns();

    double time_ns = (end - start) / 1000.0;
    double throughput = (sizes[i] * sizeof(int)) / (time_ns / 1e9) / 1e9;

    printf("%5s: %10u bytes -> %8.2f ns/elem, %.2f GB/s\n", names[i],
           (unsigned)sizes[i] * (unsigned)sizeof(int), time_ns / sizes[i],
           throughput);
  }
}

int main() {
  std::cout << "Benchmarking cache optimization techniques...\n";
  benchmark_access_patterns();
  benchmark_prefetch_optimization();
  benchmark_cache_sizes();
  std::cout << "\n✓ Benchmarks complete\n";
  return 0;
}
