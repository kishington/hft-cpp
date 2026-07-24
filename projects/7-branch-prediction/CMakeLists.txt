add_executable(test_branch test_branch.cpp)
add_executable(bench_branch bench_branch.cpp)

add_test(NAME branch_tests COMMAND test_branch)
