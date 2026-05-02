include(../../CMakeCommands.cmake)

my_add_executable(NAME explicit_mipmaps RECURSIVE)

include_directories(${Mlib_INCLUDE_DIR} ${glfw3_INCLUDE_DIR})

# MlibOpenGL contains glad_gl.cpp and glad_vulkan.cpp
target_link_libraries(explicit_mipmaps MlibOpenGL)
