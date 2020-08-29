include(../../CMakeCommands.cmake)

my_add_executable(homography_from_images_test "1")

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(homography_from_images_test MlibSfm)

add_test(NAME HomographyFromImagesTest
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../..
    COMMAND $<TARGET_FILE:homography_from_images_test>)
