include(../../CMakeCommands.cmake)

my_add_executable(NAME print_grs_info RECURSIVE)

target_link_libraries(print_grs_info MlibSceneGraph)
