include(../../CMakeCommands.cmake)

my_add_executable(NAME print_jpk_info RECURSIVE)

target_link_libraries(print_jpk_info PRIVATE MlibGeometry)
