include(../../CMakeCommands.cmake)

my_add_executable(NAME print_dff_info RECURSIVE)

target_link_libraries(print_dff_info PRIVATE MlibGeometry)
