include(../../CMakeCommands.cmake)

my_add_executable(match_histograms "1")

target_link_libraries(match_histograms MlibImages MlibStb)
