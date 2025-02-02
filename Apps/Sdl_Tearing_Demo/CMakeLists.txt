include(../../CMakeCommands.cmake)

my_add_executable(sdl_tearing_demo "1")

include_directories(${Mlib_INCLUDE_DIR} ${SDL2_INCLUDE_DIRS})

# MlibRender contains glad_gl.cpp and glad_vulkan.cpp
target_link_libraries(sdl_tearing_demo MlibRender SDL2)
