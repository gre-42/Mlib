include(../CMakeCommands.cmake)

my_add_library(
    NAME MlibNvDds
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    target_compile_options(MlibNvDds PRIVATE -Wno-sign-conversion -Wno-unused-const-variable)
endif()

# if (glfw3_FOUND)
#     target_link_libraries(MlibNvDds MlibGlad)
# endif()
target_compile_definitions(MlibNvDds PUBLIC NV_DDS_NO_GL_SUPPORT)
