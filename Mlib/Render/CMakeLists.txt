include(../../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_shared_library(MlibRender "1" "" "${mlib_excluded_files}")

include_directories(${Mlib_INCLUDE_DIR} ${glfw3_INCLUDE_DIR})

set_source_files_properties(Renderables/Renderable_Osm_Map_Rectangle.cpp PROPERTIES COMPILE_FLAGS -ffloat-store)

# Replaced with MSYS2 package
# if (MINGW)
#     target_link_libraries(MlibRender MlibImages MlibCv MlibPoly2Tri MlibSceneGraph MlibStbImage Mlib ${glfw3_LIBRARY} -lopengl32 -lglu32 -lgdi32)
# else()
#     target_link_libraries(MlibRender MlibImages MlibCv MlibPoly2Tri MlibSceneGraph MlibStbImage Mlib glfw)
# endif()

target_link_libraries(MlibRender Mlib MlibImages MlibCv MlibPoly2Tri MlibSceneGraph MlibStbImage Mlib glfw)
