include(../../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_library(MlibTime "1" "" "${mlib_excluded_files}")

target_link_libraries(MlibTime MlibOs)

include_directories(${Mlib_INCLUDE_DIR})
