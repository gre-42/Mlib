include(../../CMakeCommands.cmake)

my_add_library(
    NAME MlibThreads
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)

target_include_directories(MlibThreads PUBLIC ${Mlib_INCLUDE_DIR})

target_link_libraries(MlibThreads MlibOs MlibTime)
