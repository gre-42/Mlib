include(../../CMakeCommands.cmake)

my_add_library(
    NAME MlibResourceContext
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)

target_link_libraries(MlibResourceContext
    PUBLIC
        MlibMisc)
