include(../../CMakeCommands.cmake)

set(excluded_files "")

my_add_library(MlibLayout "1" "" "${excluded_files}")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(MlibLayout MlibOs MlibThreads MlibJson)
