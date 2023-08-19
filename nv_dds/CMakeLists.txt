include(../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_library(MlibNvDds "1" "" "${mlib_excluded_files}")

include_directories(${Mlib_INCLUDE_DIR})

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    target_compile_options(MlibNvDds PRIVATE -Wno-sign-conversion -Wno-unused-const-variable)
endif()

# if (glfw3_FOUND)
#     target_link_libraries(MlibNvDds MlibGlad)
# endif()
target_compile_definitions(MlibNvDds PUBLIC NV_DDS_NO_GL_SUPPORT)
