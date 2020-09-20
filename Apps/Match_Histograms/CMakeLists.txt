include(../../CMakeCommands.cmake)

my_add_executable(match_histograms "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(match_histograms MlibImages MlibStbImage)
