include(../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_shared_library(MlibCork "1" "" "${mlib_excluded_files}")
include_directories(mesh rawmesh math util isct accel ${CMAKE_SOURCE_DIR})
target_link_libraries(MlibCork MlibTriangle -lgmp)
