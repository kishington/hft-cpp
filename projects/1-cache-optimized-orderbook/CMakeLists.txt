add_executable(test_orderbook test_orderbook.cpp orderbook.cpp)
add_executable(bench_orderbook bench_orderbook.cpp orderbook.cpp)

add_test(NAME orderbook_tests COMMAND test_orderbook)
