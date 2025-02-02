include(../../CMakeCommands.cmake)

my_add_executable(glfw_tearing_demo "1")

include_directories(${Mlib_INCLUDE_DIR} ${glfw3_INCLUDE_DIR})

# MlibRender contains glad_gl.cpp and glad_vulkan.cpp
target_link_libraries(glfw_tearing_demo MlibRender)
