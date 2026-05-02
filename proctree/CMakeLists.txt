include(../CMakeCommands.cmake)

my_add_library(
    NAME MlibProctree
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)

target_link_libraries(MlibProctree
    MlibGeometry
    MlibJson)
