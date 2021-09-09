include(../../CMakeCommands.cmake)

my_add_executable(rof_huber_smoother "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(rof_huber_smoother MlibSfm)
