include(../../CMakeCommands.cmake)

my_add_executable(NAME match_histograms RECURSIVE)

target_link_libraries(match_histograms MlibImages MlibStb)
