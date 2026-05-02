include(../../CMakeCommands.cmake)

my_add_executable(NAME generate_tree RECURSIVE)

target_link_libraries(generate_tree PRIVATE MlibImages MlibStb)
