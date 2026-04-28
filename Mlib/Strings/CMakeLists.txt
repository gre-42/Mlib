include(../../CMakeCommands.cmake)

set(excluded_files "")

my_add_library(MlibStrings "1" "" "${excluded_files}")

target_link_libraries(MlibStrings PUBLIC MlibRegex)

# Emscripten uses "-s USE_BOOST_HEADERS=1" in the main CMakeLists.txt
if (NOT EMSCRIPTEN)
    target_link_libraries(MlibStrings PUBLIC Boost::boost)
endif()

if (WITH_ICU)
    target_link_libraries(MlibStrings PUBLIC ICU::i18n ICU::uc)
endif()

if (NOT EMSCRIPTEN)
    target_link_openmp(MlibStrings)
endif()
