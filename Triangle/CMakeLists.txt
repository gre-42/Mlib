include(../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_shared_library(MlibTriangle "1" "" "${mlib_excluded_files}")
