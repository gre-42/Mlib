include(../CMakeCommands.cmake)

my_add_library(
    NAME MlibPoly2Tri
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)

no_warn_conversion(MlibPoly2Tri)

