include(../../CMakeCommands.cmake)

my_add_executable(motion_interpolation "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(motion_interpolation MlibImages)
