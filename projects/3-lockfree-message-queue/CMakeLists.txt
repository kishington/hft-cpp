add_executable(test_queue test_queue.cpp)
add_executable(bench_queue bench_queue.cpp)

add_test(NAME queue_tests COMMAND test_queue)
