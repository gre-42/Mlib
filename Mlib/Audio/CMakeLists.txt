include(../../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_shared_library(MlibAudio "1" "" "${mlib_excluded_files}")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(MlibAudio MlibThreads ${OPENAL_LIBRARY} ${ALUT_LIBRARY} ${LIBAUDIO_LIBRARY} -laudio)
