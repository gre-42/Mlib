include(../../CMakeCommands.cmake)

my_add_library(
    NAME MlibFloat128
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)

target_include_directories(MlibFloat128 PUBLIC ${Mlib_INCLUDE_DIR})

target_link_libraries(MlibFloat128 -lquadmath)
