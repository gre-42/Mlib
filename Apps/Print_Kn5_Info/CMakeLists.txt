include(../../CMakeCommands.cmake)

my_add_executable(print_kn5_info "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(print_kn5_info MlibGeometry)
