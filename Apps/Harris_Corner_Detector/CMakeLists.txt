include(../../CMakeCommands.cmake)

my_add_executable(harris_corner_detector "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(harris_corner_detector MlibImages MlibSfm)
