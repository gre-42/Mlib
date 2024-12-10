include(../../CMakeCommands.cmake)

my_add_executable(proc_bush "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(proc_bush Mlib)

if (MINGW)
    target_link_libraries(proc_bush -lstdc++exp)
endif()
