add_library(MlibMap INTERFACE)

target_include_directories(MlibMap INTERFACE ${Mlib_INCLUDE_DIR})

target_link_libraries(MlibMap
    INTERFACE
        MlibThreads)
