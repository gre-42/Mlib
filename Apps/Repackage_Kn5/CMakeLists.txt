include(../../CMakeCommands.cmake)

my_add_executable(repackage_kn5 "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(repackage_kn5 MlibRender)
