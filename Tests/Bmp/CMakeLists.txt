include(../../CMakeCommands.cmake)

my_add_executable(NAME bmp_test RECURSIVE)

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(bmp_test MlibImages)

add_test(NAME BmpTest
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND $<TARGET_FILE:bmp_test>)
