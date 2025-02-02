include(../../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_library(MlibGeometry "1" "" "${mlib_excluded_files}")
# https://issuetracker.google.com/issues/214811726
if (NOT ANDROID)
    target_precompile_headers(MlibGeometry PRIVATE ../Precompiled_Header.hpp)
endif()

target_include_directories(
    MlibGeometry
    PUBLIC
    ${Mlib_INCLUDE_DIR}
    ${MlibCereal_INCLUDE_DIR})

set_source_files_properties(Mesh/Contour.cpp PROPERTIES COMPILE_FLAGS -ffloat-store)
set_source_files_properties(Mesh/Triangle_List.cpp PROPERTIES COMPILE_FLAGS -ffloat-store)

target_link_libraries(MlibGeometry Mlib MlibStats MlibIo MlibImages MlibMath MlibNvDds MlibJson MlibMacroExecutor MlibMemory MlibHalf)

if (BUILD_TRIANGLE)
    target_link_libraries(MlibGeometry MlibTriangle)
endif()

if (BUILD_CORK AND (NOT (MINGW OR MSVC)))
    target_link_libraries(MlibGeometry MlibCork)
endif()

if (WITH_ZLIB)
    target_link_libraries(MlibGeometry ZLIB::ZLIB)
endif()
