include(../../CMakeCommands.cmake)

my_add_executable(homography_tile_detector "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(homography_tile_detector MlibCv)
