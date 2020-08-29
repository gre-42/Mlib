include(../../CMakeCommands.cmake)

my_add_executable(array_test "1")

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(array_test stdc++fs)

add_test(NAME ArrayTest COMMAND $<TARGET_FILE:array_test>)
