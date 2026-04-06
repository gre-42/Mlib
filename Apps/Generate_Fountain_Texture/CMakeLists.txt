include(../../CMakeCommands.cmake)

my_add_executable(generate_fountain_texture "1")

target_link_libraries(generate_fountain_texture MlibImages)
