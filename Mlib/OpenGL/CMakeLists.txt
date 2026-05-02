include(../../CMakeCommands.cmake)

my_add_library(
    NAME MlibOpenGL
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)
# https://issuetracker.google.com/issues/214811726
if (NOT ANDROID)
    target_precompile_headers(MlibOpenGL PRIVATE ../Misc/Precompiled_Header.hpp)
endif()

target_include_directories(MlibOpenGL PUBLIC ${Mlib_INCLUDE_DIR} ${glfw3_INCLUDE_DIR})

target_link_libraries(MlibOpenGL
    PUBLIC
        MlibMap
        MlibGeometry
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

if (NOT EMSCRIPTEN)
    target_link_libraries(MlibOpenGL
        PUBLIC
            MlibGlad)
else()
    target_link_against_opengl(MlibOpenGL)
endif()
