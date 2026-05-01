#pragma once

#if defined(__ANDROID__)
    #include <GLES3/gl32.h>
    #define GL_CLAMP_TO_SOMETHING GL_CLAMP_TO_BORDER
#elif defined(__EMSCRIPTEN__)
    // For HTML5/WebGL 2.0. Do NOT include GLFW or GLAD.
    #include <GLES3/gl3.h>
    #include <emscripten/html5.h>
    #define GL_CLAMP_TO_BORDER 0x812D
    #define GL_CLAMP_TO_SOMETHING GL_CLAMP_TO_EDGE
    #define GL_TEXTURE_BORDER_COLOR 0x1004
    #define GL_TEXTURE_2D_MULTISAMPLE 0x9100
#else
    #include <glad/gl.h>
    #define GLFW_INCLUDE_NONE
    #include <GLFW/glfw3.h>
    #define GL_CLAMP_TO_SOMETHING GL_CLAMP_TO_BORDER
#endif
