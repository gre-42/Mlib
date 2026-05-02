include(../../CMakeCommands.cmake)

my_add_executable(NAME print_kn5_info RECURSIVE)

target_link_libraries(print_kn5_info MlibGeometry)
