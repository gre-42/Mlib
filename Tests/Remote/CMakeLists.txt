include(../../CMakeCommands.cmake)

my_add_executable(NAME remote_test RECURSIVE)

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(remote_test PRIVATE MlibRemote)

add_test(NAME RemoteTest COMMAND $<TARGET_FILE:remote_test>)
