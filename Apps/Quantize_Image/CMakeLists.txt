include(../../CMakeCommands.cmake)

my_add_executable(quantize_image "1")

target_include_directories(quantize_image PRIVATE ${Mlib_INCLUDE_DIR})

target_link_libraries(quantize_image MlibImages)
