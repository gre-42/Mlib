include(../../CMakeCommands.cmake)

set(excluded_files "")

my_add_library(MlibOpenGL "1" "" "${excluded_files}")
# https://issuetracker.google.com/issues/214811726
if (NOT ANDROID)
    target_precompile_headers(MlibOpenGL PRIVATE ../Misc/Precompiled_Header.hpp)
endif()

target_include_directories(MlibOpenGL PUBLIC ${Mlib_INCLUDE_DIR} ${glfw3_INCLUDE_DIR})

target_link_libraries(MlibOpenGL
    MlibGeometry
    MlibGlad
    MlibImages
    MlibLayout
    MlibMemory
    MlibMisc
    MlibMacroExecutor
    MlibNvDds
    MlibProctree
    MlibStats
    MlibSceneGraph
    MlibStb
    MlibTime)
