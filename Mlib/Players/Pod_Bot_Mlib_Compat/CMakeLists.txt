include(../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_library(MlibPodBotMlibCompat "1" "" "${mlib_excluded_files}")
include_directories(${Mlib_INCLUDE_DIR})
