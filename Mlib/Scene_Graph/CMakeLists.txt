include(../../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_library(MlibSceneGraph "1" "" "${mlib_excluded_files}")

target_link_libraries(MlibSceneGraph Mlib MlibMemory MlibGeometry MlibThreads)
