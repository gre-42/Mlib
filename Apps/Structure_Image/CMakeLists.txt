include(../../CMakeCommands.cmake)

my_add_executable(structure_image "1")

target_include_directories(structure_image PRIVATE ${Mlib_INCLUDE_DIR})

target_link_libraries(structure_image MlibImages)
