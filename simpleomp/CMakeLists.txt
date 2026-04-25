include(../CMakeCommands.cmake)

set(excluded_files "")

my_add_library(MlibSimpleOmp "1" "" "${excluded_files}")
target_include_directories(MlibSimpleOmp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(MlibSimpleOmp PRIVATE NCNN_SIMPLEOMP=1)
target_compile_options(MlibSimpleOmp PRIVATE -Wno-unused-variable -Wno-sign-conversion -Wno-unused-but-set-variable)
