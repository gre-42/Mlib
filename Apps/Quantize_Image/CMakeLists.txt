include(../../CMakeCommands.cmake)

my_add_executable(NAME quantize_image RECURSIVE)

target_include_directories(quantize_image PRIVATE ${Mlib_INCLUDE_DIR})

target_link_libraries(quantize_image MlibImages)
