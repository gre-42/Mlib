include(../../CMakeCommands.cmake)

my_add_executable(NAME generate_fountain_texture RECURSIVE)

target_link_libraries(generate_fountain_texture MlibImages)
