include(../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_library(MlibStb "1" "" "${mlib_excluded_files}")

target_compile_options(MlibStb PRIVATE -Wno-conversion)

include_directories(${Mlib_INCLUDE_DIR})
