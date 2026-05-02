include(../../CMakeCommands.cmake)

my_add_library(
    NAME MlibStrings
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)

target_include_directories(MlibStrings PUBLIC ${Mlib_INCLUDE_DIR})

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
