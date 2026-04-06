include(../../CMakeCommands.cmake)

set(excluded_files "")

my_add_library(MlibGeometry "1" "" "${excluded_files}")

target_include_directories(MlibGeometry PUBLIC ${Mlib_INCLUDE_DIR} ${MlibCereal_INCLUDE_DIR})

target_link_libraries(MlibGeometry
    MlibMath
    MlibHalf
    MlibImages
    MlibMacroExecutor
    MlibCompression)
