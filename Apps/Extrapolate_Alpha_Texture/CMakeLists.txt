include(../../CMakeCommands.cmake)

my_add_executable(NAME extrapolate_alpha_texture RECURSIVE)

target_link_libraries(extrapolate_alpha_texture PRIVATE MlibImages)
