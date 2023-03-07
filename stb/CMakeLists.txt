include(../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_library(MlibStb "1" "" "${mlib_excluded_files}")

no_warn_conversion(MlibStb)

include_directories(${Mlib_INCLUDE_DIR})
