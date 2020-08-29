include(../../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_shared_library(MlibScene "1" "" "${mlib_excluded_files}")

include_directories(${Mlib_INCLUDE_DIR} ${glfw3_INCLUDE_DIR})

target_link_libraries(MlibScene MlibPhysics MlibRender stdc++fs)
