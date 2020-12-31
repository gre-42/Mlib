include(../../CMakeCommands.cmake)

my_add_executable(local_polynomial_regression "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(local_polynomial_regression MlibImages)
