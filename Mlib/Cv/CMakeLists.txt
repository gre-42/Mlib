include(../../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_library(MlibCv "1" "" "${mlib_excluded_files}")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(MlibCv MlibGeometry MlibImages)

if (glfw3_FOUND OR ANDROID)
    target_link_libraries(MlibCv MlibRender)
endif()
