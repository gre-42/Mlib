include(../../CMakeCommands.cmake)

my_add_executable(proc_bush_svg "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(proc_bush_svg Mlib)
