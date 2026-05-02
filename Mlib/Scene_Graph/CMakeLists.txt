include(../../CMakeCommands.cmake)

my_add_library(
    NAME MlibSceneGraph
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)

target_include_directories(MlibSceneGraph PUBLIC ${Mlib_INCLUDE_DIR})

target_link_libraries(MlibSceneGraph
    PUBLIC
        MlibMap
        MlibGeometry
        MlibRegex)
