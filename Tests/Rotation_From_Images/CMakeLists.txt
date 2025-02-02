include(../../CMakeCommands.cmake)

my_add_executable(rotation_from_images_test "1")

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(rotation_from_images_test MlibSfm)

add_test(NAME RotationFromImagesTest
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND $<TARGET_FILE:rotation_from_images_test>)
