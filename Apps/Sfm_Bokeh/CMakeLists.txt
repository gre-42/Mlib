include(../../CMakeCommands.cmake)

my_add_executable(sfm_bokeh "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(sfm_bokeh MlibSfm)
