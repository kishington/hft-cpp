add_executable(test_ringbuffer test_ringbuffer.cpp)
add_executable(bench_ringbuffer bench_ringbuffer.cpp)

add_test(NAME ringbuffer_tests COMMAND test_ringbuffer)
