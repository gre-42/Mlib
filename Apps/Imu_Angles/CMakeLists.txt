include(../../CMakeCommands.cmake)

my_add_executable(NAME imu_angles RECURSIVE)

target_link_libraries(imu_angles PRIVATE MlibMath)
