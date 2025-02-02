include(../../CMakeCommands.cmake)

my_add_executable(rigid_motion_from_images_test "1")

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(rigid_motion_from_images_test MlibSfm)

add_test(NAME RigidMotionFromImagesTest
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND $<TARGET_FILE:rigid_motion_from_images_test>)
