include(../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_shared_library(MlibPoly2Tri "1" "" "${mlib_excluded_files}")
