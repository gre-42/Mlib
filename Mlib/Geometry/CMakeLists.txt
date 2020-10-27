include(../../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_shared_library(MlibGeometry "1" "" "${mlib_excluded_files}")

include_directories(${Mlib_INCLUDE_DIR})

set_source_files_properties(Mesh/Contour.cpp PROPERTIES COMPILE_FLAGS -ffloat-store)
set_source_files_properties(Mesh/Triangle_List.cpp PROPERTIES COMPILE_FLAGS -ffloat-store)

target_link_libraries(MlibGeometry MlibImages MlibMath stdc++fs)
