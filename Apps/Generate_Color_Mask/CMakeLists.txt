include(../../CMakeCommands.cmake)

my_add_executable(NAME generate_color_mask RECURSIVE)

target_link_libraries(generate_color_mask MlibImages)
