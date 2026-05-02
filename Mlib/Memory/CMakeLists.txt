include(../../CMakeCommands.cmake)

my_add_library(
    NAME MlibMemory
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)

target_include_directories(MlibMemory PUBLIC ${Mlib_INCLUDE_DIR})

target_link_libraries(MlibMemory PUBLIC MlibOs MlibMisc MlibTime)
