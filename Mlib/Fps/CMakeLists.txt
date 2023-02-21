include(../../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_library(MlibFps "1" "" "${mlib_excluded_files}")

target_link_libraries(MlibFps MlibOs)

include_directories(${Mlib_INCLUDE_DIR})
