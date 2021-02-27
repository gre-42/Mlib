include(../../CMakeCommands.cmake)

my_add_executable(normalize_brightness "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(normalize_brightness MlibImages)
