include(../../CMakeCommands.cmake)

my_add_executable(NAME apply_normal_map RECURSIVE)

include_directories(${Mlib_INCLUDE_DIR} ${glfw3_INCLUDE_DIR})

# MlibOpenGL contains glad_gl.cpp and glad_vulkan.cpp
target_link_libraries(apply_normal_map PRIVATE MlibOpenGL)
