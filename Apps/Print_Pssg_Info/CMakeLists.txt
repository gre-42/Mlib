include(../../CMakeCommands.cmake)

my_add_executable(NAME print_pssg_info RECURSIVE)

target_link_libraries(print_pssg_info MlibGeometry)
