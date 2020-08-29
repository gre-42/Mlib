include(../../CMakeCommands.cmake)

my_add_executable(registration_test "1")

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(registration_test MlibImages)

add_test(NAME RegistrationTest COMMAND $<TARGET_FILE:registration_test>)
