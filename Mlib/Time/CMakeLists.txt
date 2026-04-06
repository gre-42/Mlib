include(../../CMakeCommands.cmake)

set(excluded_files "")

my_add_library(MlibTime "1" "" "${excluded_files}")

target_link_libraries(MlibTime MlibOs)

target_include_directories(MlibTime PUBLIC ${Mlib_INCLUDE_DIR})
