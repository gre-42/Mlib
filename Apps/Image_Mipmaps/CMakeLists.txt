include(../../CMakeCommands.cmake)

my_add_executable(NAME image_mipmaps RECURSIVE)

include_directories(${Mlib_INCLUDE_DIR} ${glfw3_INCLUDE_DIR})

target_link_libraries(image_mipmaps MlibStbCpp)
