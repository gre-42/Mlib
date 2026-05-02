include(../../CMakeCommands.cmake)

my_add_library(
    NAME MlibStats
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)

target_include_directories(MlibStats PUBLIC ${Mlib_INCLUDE_DIR})

target_link_libraries(MlibStats MlibOs)
