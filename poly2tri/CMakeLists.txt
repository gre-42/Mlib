include(../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_library(MlibPoly2Tri "1" "" "${mlib_excluded_files}")

no_warn_conversion(MlibPoly2Tri)

