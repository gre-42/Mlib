include(../../CMakeCommands.cmake)

my_add_executable(render_texture "1")

include_directories(${Mlib_INCLUDE_DIR} ${glfw3_INCLUDE_DIR})

target_link_libraries(render_texture MlibRender)
