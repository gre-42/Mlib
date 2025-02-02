include(../../CMakeCommands.cmake)

my_add_executable(cgi_player "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(cgi_player Mlib)
