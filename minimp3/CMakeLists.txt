include(../CMakeCommands.cmake)

my_add_library(
    NAME MlibMiniMp3
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)

target_link_libraries(MlibMiniMp3 PUBLIC MlibMisc)
