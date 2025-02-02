include(../../CMakeCommands.cmake)

my_add_executable(sparse_reconstruction_test "1")

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(sparse_reconstruction_test MlibSfm)

add_test(NAME SparseReconstructionTest
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND $<TARGET_FILE:sparse_reconstruction_test>)
