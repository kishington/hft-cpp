add_executable(test_numa test_numa.cpp)
add_executable(bench_numa bench_numa.cpp)

add_test(NAME numa_tests COMMAND test_numa)
