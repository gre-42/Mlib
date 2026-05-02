include(../../CMakeCommands.cmake)

my_add_executable(NAME sdl_tearing_demo RECURSIVE)

include_directories(${Mlib_INCLUDE_DIR} ${SDL2_INCLUDE_DIRS})

# MlibOpenGL contains glad_gl.cpp and glad_vulkan.cpp
target_link_libraries(sdl_tearing_demo PRIVATE MlibOpenGL SDL2)
