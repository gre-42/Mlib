include(../CMakeCommands.cmake)

my_add_library(
    NAME MlibSimpleOmp
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)
target_include_directories(MlibSimpleOmp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(MlibSimpleOmp PRIVATE NCNN_SIMPLEOMP=1)
target_compile_options(MlibSimpleOmp PRIVATE -Wno-unused-variable -Wno-sign-conversion -Wno-unused-but-set-variable)
