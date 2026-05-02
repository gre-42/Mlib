include(../../CMakeCommands.cmake)

my_add_executable(NAME cfd_test RECURSIVE)

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(cfd_test MlibMisc MlibPhysics)

add_test(NAME CfdTest COMMAND $<TARGET_FILE:cfd_test>)
