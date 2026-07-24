#include "cache.hpp"
#include <cassert>
#include <iostream>
#include <cstring>

void test_cache_alignment() {
  CacheAlignedArray<int> arr(1000);
  assert(arr.get_size() == 1000);

  // Write and read
  for (size_t i = 0; i < 1000; i++) {
    arr[i] = i * 2;
  }

  for (size_t i = 0; i < 1000; i++) {
    assert(arr[i] == (int)(i * 2));
  }

  std::cout << "✓ Cache alignment test passed\n";
}

void test_sequential_access() {
  CacheAlignedArray<int> arr(10000);
  for (size_t i = 0; i < arr.get_size(); i++) {
    arr[i] = i;
  }

  SequentialAccessor<int>::access_pattern(arr.data_ptr(), arr.get_size());

  std::cout << "✓ Sequential access test passed\n";
}

void test_strided_access() {
  CacheAlignedArray<int> arr(10000);
  for (size_t i = 0; i < arr.get_size(); i++) {
    arr[i] = i;
  }

  StridedAccessor<int> accessor(64);  // Large stride
  accessor.access_pattern(arr.data_ptr(), arr.get_size());

  std::cout << "✓ Strided access test passed\n";
}

void test_prefetch() {
  CacheAlignedArray<int> arr(10000);
  for (size_t i = 0; i < arr.get_size(); i++) {
    arr[i] = i;
  }

  PrefetchedAccess<int>::access_with_prefetch(arr.data_ptr(),
                                              arr.get_size());

  std::cout << "✓ Prefetch test passed\n";
}

int main() {
  test_cache_alignment();
  test_sequential_access();
  test_strided_access();
  test_prefetch();
  std::cout << "\nAll cache tests passed!\n";
  return 0;
}
