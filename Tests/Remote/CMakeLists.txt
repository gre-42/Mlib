include(../../CMakeCommands.cmake)

my_add_executable(remote_test "1")

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(remote_test MlibRemote)

add_test(NAME RemoteTest COMMAND $<TARGET_FILE:remote_test>)
