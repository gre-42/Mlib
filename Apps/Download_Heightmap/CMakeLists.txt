include(../../CMakeCommands.cmake)

my_add_executable(download_heightmap "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(download_heightmap Mlib MlibImages MlibStbImage MlibCppHttplib)
