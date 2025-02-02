include(../../CMakeCommands.cmake)

my_add_executable(schur_complement_test "1")

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(schur_complement_test MlibSfm)

add_test(NAME SchurComplementTest
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND $<TARGET_FILE:schur_complement_test>)
