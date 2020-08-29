include(../../CMakeCommands.cmake)

my_add_executable(sfm_dtam_pipeline "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(sfm_dtam_pipeline MlibSfm)
