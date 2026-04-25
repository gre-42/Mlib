if (ANDROID)
    add_library(AGameHelper
        STATIC
            Android/AContext.cpp
            Android/AWindow.cpp
            Android/ARenderLoop.cpp
            Android/AEngine.cpp
            Android/AssetUtil.cpp
    )
elseif (EMSCRIPTEN)
    add_library(AGameHelper
        STATIC
            Emscripten/AContext.cpp
            Emscripten/ARenderLoop.cpp
            Emscripten/AEngine.cpp
    )
else()
    message(FATAL_ERROR "Unknown platform type")
endif()

target_link_libraries(AGameHelper PUBLIC MlibOs)
