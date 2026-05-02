include(../../CMakeCommands.cmake)

my_add_executable(NAME render_obj_file RECURSIVE)

target_link_libraries(render_obj_file MlibOpenGL)
