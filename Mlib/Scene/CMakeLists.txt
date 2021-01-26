include(../../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_shared_library(MlibScene "1" "" "${mlib_excluded_files}")

include_directories(${Mlib_INCLUDE_DIR} ${glfw3_INCLUDE_DIR})

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set (fs_lib stdc++fs)
else()
    set (fs_lib "")
endif()
target_link_libraries(MlibScene MlibPhysics MlibRender ${fs_lib})
