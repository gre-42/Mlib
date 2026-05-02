include(../CMakeCommands.cmake)

my_add_library(
    NAME MlibStbCpp
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)
target_include_directories(MlibStbCpp PUBLIC ${Mlib_INCLUDE_DIR})
target_link_libraries(MlibStbCpp PUBLIC MlibStb MlibMisc MlibArray)
