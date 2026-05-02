include(../../CMakeCommands.cmake)

my_add_library(
    NAME MlibTime
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)

target_link_libraries(MlibTime
    PUBLIC
        MlibHeader
        MlibOs)
