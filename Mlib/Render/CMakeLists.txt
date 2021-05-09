include(../../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_shared_library(MlibRender "1" "" "${mlib_excluded_files}")

include_directories(${Mlib_INCLUDE_DIR} ${glfw3_INCLUDE_DIR})

# Replaced with MSYS2 package
# if (MINGW)
#     target_link_libraries(MlibRender MlibImages MlibCv MlibPoly2Tri MlibSceneGraph MlibStbImage Mlib ${glfw3_LIBRARY} -lopengl32 -lglu32 -lgdi32)
# else()
#     target_link_libraries(MlibRender MlibImages MlibCv MlibPoly2Tri MlibSceneGraph MlibStbImage Mlib glfw)
# endif()

if (CMAKE_GENERATOR MATCHES "Visual Studio")
    target_link_libraries(MlibRender Mlib MlibImages MlibCv MlibPoly2Tri MlibSceneGraph MlibStbImage Mlib ${glfw3_LIBRARY})
else()
    target_link_libraries(MlibRender Mlib MlibImages MlibCv MlibPoly2Tri MlibSceneGraph MlibStbImage Mlib glfw)
endif()

if (MSVC)
    target_compile_definitions(MlibRender PUBLIC GLAD_API_CALL_EXPORT)
    target_compile_definitions(MlibRender PRIVATE GLAD_API_CALL_EXPORT_BUILD)
endif()
