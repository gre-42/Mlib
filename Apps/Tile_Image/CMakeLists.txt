include(../../CMakeCommands.cmake)

my_add_executable(tile_image "1")

target_include_directories(tile_image PRIVATE ${Mlib_INCLUDE_DIR})

target_link_libraries(tile_image MlibImages)
