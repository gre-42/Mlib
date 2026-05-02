include(../CMakeCommands.cmake)

my_add_library(
    NAME MlibCork
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)
include_directories(mesh rawmesh math util isct accel ${CMAKE_SOURCE_DIR})
target_link_libraries(MlibCork PUBLIC MlibTriangle -lgmp)
