add_executable(test_cache test_cache.cpp)
add_executable(bench_cache bench_cache.cpp)

add_test(NAME cache_tests COMMAND test_cache)
