include(../../CMakeCommands.cmake)

my_add_executable(intrinsic_matrix_from_dimensions "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(intrinsic_matrix_from_dimensions MlibCv)
