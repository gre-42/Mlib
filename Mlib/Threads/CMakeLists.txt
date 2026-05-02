include(../../CMakeCommands.cmake)

my_add_library(
    NAME MlibThreads
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)

target_link_libraries(MlibThreads
    PUBLIC
        MlibOs
        MlibTime)
