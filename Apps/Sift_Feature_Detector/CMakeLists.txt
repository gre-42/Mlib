include(../../CMakeCommands.cmake)

my_add_executable(NAME sift_feature_detector RECURSIVE)

target_link_libraries(sift_feature_detector MlibImages MlibSfm)
