include(../../CMakeCommands.cmake)

my_add_executable(enhance_window_texture "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(enhance_window_texture Mlib MlibStbImage)
