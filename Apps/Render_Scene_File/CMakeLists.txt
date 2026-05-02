include(../../CMakeCommands.cmake)

my_add_executable(NAME render_scene_file RECURSIVE)

target_include_directories(render_scene_file PRIVATE ${Mlib_INCLUDE_DIR} ${glfw3_INCLUDE_DIR})

target_link_libraries(render_scene_file MlibScene)
