include(../../CMakeCommands.cmake)

my_add_library(
    NAME MlibJson
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)

target_include_directories(MlibJson PUBLIC ${Mlib_INCLUDE_DIR})

target_link_libraries(MlibJson PUBLIC MlibMisc MlibOs)
