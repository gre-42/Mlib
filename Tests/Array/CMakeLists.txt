include(../../CMakeCommands.cmake)

my_add_executable(array_test "1")

include_directories(Mlib ${Mlib_INCLUDE_DIR})

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set (fs_lib stdc++fs)
else()
    set (fs_lib "")
endif()
target_link_libraries(array_test ${fs_lib})

add_test(NAME ArrayTest COMMAND $<TARGET_FILE:array_test>)
