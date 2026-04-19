include(../CMakeCommands.cmake)

set(excluded_files "")

my_add_library(MlibMiniMp3 "1" "" "${excluded_files}")

target_link_libraries(MlibMiniMp3 MlibMisc)
