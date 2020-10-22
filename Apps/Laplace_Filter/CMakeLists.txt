include(../../CMakeCommands.cmake)

my_add_executable(laplace_filter "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(laplace_filter MlibImages)
