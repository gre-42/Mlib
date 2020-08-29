include(../../CMakeCommands.cmake)

my_add_executable(naive_bayes_regression_test "1")

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(naive_bayes_regression_test MlibMath)

add_test(NAME NaiveBayesRegressionTest COMMAND $<TARGET_FILE:naive_bayes_regression_test>)
