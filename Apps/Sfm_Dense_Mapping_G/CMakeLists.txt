include(../../CMakeCommands.cmake)

my_add_executable(sfm_dense_mapping_g "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(sfm_dense_mapping_g MlibSfm)
