include(../../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_library(MlibAudio "1" "" "${mlib_excluded_files}")

target_include_directories(MlibAudio PUBLIC ${Mlib_INCLUDE_DIR} ${ALUT_INCLUDE_DIRS})

target_link_libraries(MlibAudio Mlib MlibThreads MlibMemory MlibTime MlibSceneGraph ${ALUT_LIBRARIES})
