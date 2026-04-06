include(../../CMakeCommands.cmake)

set(excluded_files "")

my_add_library(MlibSceneGraph "1" "" "${excluded_files}")

target_include_directories(MlibSceneGraph PUBLIC ${Mlib_INCLUDE_DIR})

target_link_libraries(MlibSceneGraph MlibGeometry MlibRegex)
