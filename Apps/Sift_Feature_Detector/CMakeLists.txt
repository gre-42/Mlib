include(../../CMakeCommands.cmake)

my_add_executable(sift_feature_detector "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(sift_feature_detector MlibImages MlibSfm)
