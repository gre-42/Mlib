include(../../CMakeCommands.cmake)

my_add_executable(extrapolate_alpha_texture "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(extrapolate_alpha_texture MlibImages)
