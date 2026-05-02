include(../../CMakeCommands.cmake)

my_add_library(
    NAME MlibArray
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)

target_include_directories(MlibArray PUBLIC ${Mlib_INCLUDE_DIR})

target_link_libraries(MlibArray MlibIo)
