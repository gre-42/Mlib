include(../../CMakeCommands.cmake)

set(excluded_files "")

my_add_library(MlibStrings "1" "" "${excluded_files}")

target_link_libraries(MlibStrings PUBLIC MlibRegex)

if (NOT EMSCRIPTEN)
    target_link_libraries(MlibStrings PUBLIC Boost::boost)
endif()

if (WITH_ICU)
    target_link_libraries(MlibStrings PUBLIC ICU::i18n ICU::uc)
endif()

if (EMSCRIPTEN)
    target_link_openmp(MlibStrings)
endif()
