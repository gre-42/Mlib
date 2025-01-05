#include "Generic_Post_Processing_Logic.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Shader_Version_3_0.hpp>

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
    : quad_vertices_ { quad_vertices }
{
    va_.add_array_buffer(vertices_);
}

GenericPostProcessingLogic::~GenericPostProcessingLogic() = default;

VertexArray& GenericPostProcessingLogic::va() {
    if (!va_.initialized()) {
        // screen quad VAO
        va_.initialize();
        vertices_.set(quad_vertices_, quad_vertices_ + 4 * 6, TaskLocation::FOREGROUND);
        CHK(glEnableVertexAttribArray(0));
        CHK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr));
        CHK(glEnableVertexAttribArray(1));
        CHK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))));
        CHK(glBindVertexArray(0));
    }
    return va_;
}
