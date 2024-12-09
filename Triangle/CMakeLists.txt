include(../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_library(MlibTriangle "1" "" "${mlib_excluded_files}")

no_warn_conversion(MlibTriangle)

if (MSVC)
    target_compile_definitions(MlibTriangle PRIVATE NO_TIMER)
endif()
