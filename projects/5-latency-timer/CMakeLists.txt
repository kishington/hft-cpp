add_executable(test_timer test_timer.cpp)
add_executable(bench_timer bench_timer.cpp)

add_test(NAME timer_tests COMMAND test_timer)
