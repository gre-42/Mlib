include(../../CMakeCommands.cmake)

my_add_executable(NAME misc_test RECURSIVE)

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(misc_test
    PRIVATE
        MlibMap
        MlibMisc
        MlibMath
        MlibRegex
        MlibThreads
        MlibMemory)

add_test(NAME MiscTest COMMAND $<TARGET_FILE:misc_test>)
