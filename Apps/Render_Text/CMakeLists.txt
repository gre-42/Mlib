include(../../CMakeCommands.cmake)

my_add_executable(NAME render_text RECURSIVE)

include_directories(${Mlib_INCLUDE_DIR} ${glfw3_INCLUDE_DIR})

target_link_libraries(render_text PRIVATE MlibOpenGL)
