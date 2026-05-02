include(../../CMakeCommands.cmake)

my_add_executable(NAME print_gamepad_buttons RECURSIVE)

include_directories(${Mlib_INCLUDE_DIR} ${glfw3_INCLUDE_DIR})

target_link_libraries(print_gamepad_buttons MlibOpenGL)
