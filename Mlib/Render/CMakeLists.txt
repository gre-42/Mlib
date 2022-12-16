include(../../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_library(MlibRender "1" "" "${mlib_excluded_files}")

target_include_directories(MlibRender PUBLIC ${Mlib_INCLUDE_DIR})
if (glfw3_FOUND)
    target_include_directories(MlibRender PUBLIC ${glfw3_INCLUDE_DIR})
endif()

# Replaced with MSYS2 package
# if (MINGW)
#     target_link_libraries(MlibRender MlibImages MlibSceneGraph MlibStbImage Mlib ${glfw3_LIBRARY} -lopengl32 -lglu32 -lgdi32)
# else()
#     target_link_libraries(MlibRender MlibImages MlibSceneGraph MlibStbImage Mlib glfw)
# endif()

target_link_libraries(MlibRender Mlib MlibImages MlibSceneGraph MlibStbImage MlibFps)

if (glfw3_FOUND)
    if (CMAKE_GENERATOR MATCHES "Visual Studio")
        target_link_libraries(MlibRender ${glfw3_LIBRARY})
    else()
        target_link_libraries(MlibRender glfw)
    endif()
endif()

if (ANDROID)
    target_link_libraries(MlibRender GLESv3 EGL)
endif()

if (MSVC)
    target_compile_definitions(MlibRender PUBLIC GLAD_API_CALL_EXPORT)
    target_compile_definitions(MlibRender PRIVATE GLAD_API_CALL_EXPORT_BUILD)
endif()
