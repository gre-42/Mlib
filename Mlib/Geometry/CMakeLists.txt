include(../../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_shared_library(MlibGeometry "1" "" "${mlib_excluded_files}")

include_directories(${Mlib_INCLUDE_DIR})

set_source_files_properties(Mesh/Contour.cpp PROPERTIES COMPILE_FLAGS -ffloat-store)
set_source_files_properties(Mesh/Triangle_List.cpp PROPERTIES COMPILE_FLAGS -ffloat-store)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set (fs_lib stdc++fs)
else()
    set (fs_lib "")
endif()
target_link_libraries(MlibGeometry MlibImages MlibMath MlibCork ${fs_lib})
