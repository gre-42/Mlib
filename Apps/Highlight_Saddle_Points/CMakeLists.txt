include(../../CMakeCommands.cmake)

my_add_executable(highlight_saddle_points "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(highlight_saddle_points MlibImages)
