include(../CMakeCommands.cmake)

set(excluded_files "")

my_add_library(MlibProctree "1" "" "${excluded_files}")

target_link_libraries(MlibProctree
    MlibGeometry
    MlibJson)
