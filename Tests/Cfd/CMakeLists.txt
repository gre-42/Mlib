include(../../CMakeCommands.cmake)

my_add_executable(cfd_test "1")

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(cfd_test Mlib MlibPhysics)

add_test(NAME CfdTest COMMAND $<TARGET_FILE:cfd_test>)
