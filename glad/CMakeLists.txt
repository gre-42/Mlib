include(../CMakeCommands.cmake)

my_add_library(
    NAME MlibGlad
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)

target_link_against_opengl(MlibGlad)
target_link_libraries(MlibGlad MlibHeader)
