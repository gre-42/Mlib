include(../../CMakeCommands.cmake)

my_add_executable(NAME local_polynomial_regression RECURSIVE)

target_link_libraries(local_polynomial_regression PRIVATE MlibImages)
