include(../../CMakeCommands.cmake)

my_add_library(
    NAME MlibLayout
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)

target_link_libraries(MlibLayout MlibOs MlibThreads MlibJson)
