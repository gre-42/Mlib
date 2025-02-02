include(../../CMakeCommands.cmake)

my_add_executable(images_test "1")

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(images_test MlibImages)

add_test(NAME ImagesTest
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND $<TARGET_FILE:images_test>)
