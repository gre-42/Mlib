include(../../CMakeCommands.cmake)

my_add_executable(NAME structure_image RECURSIVE)

target_include_directories(structure_image PRIVATE ${Mlib_INCLUDE_DIR})

target_link_libraries(structure_image MlibImages)
