include(../../CMakeCommands.cmake)

my_add_library(
    NAME MlibMisc
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)

target_include_directories(MlibMisc PUBLIC ${Mlib_INCLUDE_DIR})

target_link_libraries(MlibMisc MlibStrings MlibOs MlibThreads)
