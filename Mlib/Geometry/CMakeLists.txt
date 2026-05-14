include(../../CMakeCommands.cmake)

my_add_library(
    NAME MlibGeometry
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)

target_include_directories(MlibGeometry PUBLIC ${Mlib_INCLUDE_DIR})

target_link_libraries(MlibGeometry
    PUBLIC
        MlibMap
        MlibMath
        MlibHalf
        MlibImages
        MlibMacroExecutor
        MlibCompression
        MlibCereal)
