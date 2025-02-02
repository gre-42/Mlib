include(../../CMakeCommands.cmake)

my_add_executable(render_point_cloud "1")

include_directories(${Mlib_INCLUDE_DIR} ${glfw3_INCLUDE_DIR})

target_link_libraries(render_point_cloud MlibRender MlibCv)
