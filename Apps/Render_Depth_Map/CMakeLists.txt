include(../../CMakeCommands.cmake)

my_add_executable(render_depth_map "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(render_depth_map MlibRender MlibCv)
