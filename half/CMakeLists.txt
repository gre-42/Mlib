include(../CMakeCommands.cmake)

my_add_library(
    NAME MlibHalf
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    target_compile_options(MlibHalf PRIVATE -Wno-sign-conversion -Wno-unused-function)
endif()
