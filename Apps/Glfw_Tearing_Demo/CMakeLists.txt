include(../../CMakeCommands.cmake)

my_add_executable(NAME glfw_tearing_demo RECURSIVE)

include_directories(${Mlib_INCLUDE_DIR} ${glfw3_INCLUDE_DIR})

# MlibOpenGL contains glad_gl.cpp and glad_vulkan.cpp
target_link_libraries(glfw_tearing_demo MlibOpenGL)
