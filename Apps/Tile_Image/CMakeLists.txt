include(../../CMakeCommands.cmake)

my_add_executable(NAME tile_image RECURSIVE)

target_include_directories(tile_image PRIVATE ${Mlib_INCLUDE_DIR})

target_link_libraries(tile_image PRIVATE MlibImages)
