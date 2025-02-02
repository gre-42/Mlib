include(../../CMakeCommands.cmake)

my_add_executable(proc_tree "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(proc_tree Mlib)

if (MINGW)
    target_link_libraries(proc_tree -lstdc++exp)
endif()
