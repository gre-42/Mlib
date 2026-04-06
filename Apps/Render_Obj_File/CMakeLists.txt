include(../../CMakeCommands.cmake)

my_add_executable(render_obj_file "1")

target_link_libraries(render_obj_file MlibOpenGL)
