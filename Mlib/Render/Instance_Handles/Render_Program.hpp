#pragma once
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace Mlib {

struct RenderProgram {
    RenderProgram() = default;
    RenderProgram(const RenderProgram&) = delete;
    RenderProgram& operator = (const RenderProgram&) = delete;
    ~RenderProgram();
    GLuint vertex_shader = (GLuint)-1;
    GLuint fragment_shader = (GLuint)-1;
    GLuint program = (GLuint)-1;
    void generate(const char * vertex_shader_text, const char * fragment_shader_text);
    void free();
};

}
