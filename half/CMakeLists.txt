include(../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_library(MlibHalf "1" "" "${mlib_excluded_files}")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    target_compile_options(MlibHalf PRIVATE -Wno-sign-conversion -Wno-unused-function)
endif()
