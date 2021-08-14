include(../../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_shared_library(MlibSfm "1" "" "${mlib_excluded_files}")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(MlibSfm MlibCv)

if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    target_link_libraries(MlibSfm stdc++fs)
endif()

if (OpenCV_FOUND)
    target_link_libraries(MlibSfm ${OpenCV_LIBS})
endif()
