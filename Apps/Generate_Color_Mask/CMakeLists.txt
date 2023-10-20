include(../../CMakeCommands.cmake)

my_add_executable(generate_color_mask "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(generate_color_mask MlibImages)
