#include "Generic_Post_Processing_Logic.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Shader_Version.hpp>

using namespace Mlib;

const char* GenericPostProcessingLogic::simple_vertex_shader_text_ =
SHADER_VER
"layout (location = 0) in vec2 aPos;\n"
"layout (location = 1) in vec2 aTexCoords;\n"
"\n"
"out vec2 TexCoords;\n"
"\n"
"void main()\n"
"{\n"
"    TexCoords = aTexCoords;\n"
"    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
"}";

GenericPostProcessingLogic::GenericPostProcessingLogic(const float* quad_vertices)
: quad_vertices_{quad_vertices}
{}

GenericPostProcessingLogic::~GenericPostProcessingLogic()
{}

VertexArray& GenericPostProcessingLogic::va() {
    if (va_.vertex_array == (GLuint)-1) {
        // screen quad VAO
        CHK(glGenVertexArrays(1, &va_.vertex_array));
        CHK(glGenBuffers(1, &va_.vertex_buffer));
        CHK(glBindVertexArray(va_.vertex_array));
        CHK(glBindBuffer(GL_ARRAY_BUFFER, va_.vertex_buffer));
        CHK(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 6, quad_vertices_, GL_STATIC_DRAW));
        CHK(glEnableVertexAttribArray(0));
        CHK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0));
        CHK(glEnableVertexAttribArray(1));
        CHK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))));
        CHK(glBindVertexArray(0));
    }
    return va_;
}
