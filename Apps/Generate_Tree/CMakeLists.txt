include(../../CMakeCommands.cmake)

my_add_executable(generate_tree "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(generate_tree MlibImages MlibStbImage)
