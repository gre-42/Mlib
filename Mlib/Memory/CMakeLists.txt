include(../../CMakeCommands.cmake)

set(excluded_files "")

my_add_library(MlibMemory "1" "" "${excluded_files}")

target_include_directories(MlibMemory PUBLIC ${Mlib_INCLUDE_DIR})

target_link_libraries(MlibMemory MlibOs MlibMisc)
