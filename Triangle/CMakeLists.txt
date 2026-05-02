include(../CMakeCommands.cmake)

my_add_library(
    NAME MlibTriangle
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)

no_warn_conversion(MlibTriangle)

if (MSVC)
    target_compile_definitions(MlibTriangle PRIVATE NO_TIMER)
endif()
