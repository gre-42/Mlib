include(../../CMakeCommands.cmake)

set(excluded_files "")

my_add_library(MlibCompression "1" "" "${excluded_files}")

target_link_libraries(MlibCompression MlibIo MlibOs)

if (WITH_ZLIB)
    target_link_libraries(MlibCompression ZLIB::ZLIB)
endif()
