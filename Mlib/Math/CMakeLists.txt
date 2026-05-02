include(../../CMakeCommands.cmake)

my_add_library(
    NAME MlibMath
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)

target_include_directories(MlibMath PUBLIC ${Mlib_INCLUDE_DIR})

target_link_libraries(MlibMath PUBLIC MlibOs MlibArray MlibJson)
