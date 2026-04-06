include(../../CMakeCommands.cmake)

set(excluded_files "")

my_add_library(MlibThreads "1" "" "${excluded_files}")

target_include_directories(MlibThreads PUBLIC ${Mlib_INCLUDE_DIR})

target_link_libraries(MlibThreads MlibOs MlibTime)
