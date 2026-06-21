include(../../CMakeCommands.cmake)

my_add_executable(NAME serialize_test RECURSIVE)

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(serialize_test
    PRIVATE MlibIo)

add_test(NAME SerializeTest COMMAND $<TARGET_FILE:serialize_test>)
