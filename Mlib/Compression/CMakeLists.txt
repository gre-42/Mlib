include(../../CMakeCommands.cmake)

my_add_library(
    NAME MlibCompression
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)

target_link_libraries(MlibCompression PUBLIC MlibIo MlibOs)

if (WITH_ZLIB AND NOT EMSCRIPTEN)
    target_link_libraries(MlibCompression PUBLIC ZLIB::ZLIB)
endif()
