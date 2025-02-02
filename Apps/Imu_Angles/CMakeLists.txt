include(../../CMakeCommands.cmake)

my_add_executable(imu_angles "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(imu_angles MlibMath)
