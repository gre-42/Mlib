include(../../CMakeCommands.cmake)

my_add_executable(array_test "1")

include_directories(Mlib ${Mlib_INCLUDE_DIR})

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries (array_test stdc++fs)
endif()
target_link_libraries(array_test MlibStrings)

add_test(NAME ArrayTest COMMAND $<TARGET_FILE:array_test>)
