include(../CMakeCommands.cmake)

my_add_library(
    NAME MlibStb
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)

no_warn_conversion(MlibStb)

target_link_libraries(MlibStb MlibHeader)
target_link_math(MlibStb)
