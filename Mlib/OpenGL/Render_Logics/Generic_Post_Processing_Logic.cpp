#include "Generic_Post_Processing_Logic.hpp"
#include <Mlib/OpenGL/CHK.hpp>
#include <Mlib/OpenGL/Gen_Shader_Text.hpp>
#include <Mlib/OpenGL/Shader_Version_3_0.hpp>
#include <sstream>

using namespace Mlib;

const char* GenericPostProcessingLogic::simple_vertex_shader_text() {
    static GenShaderText result = [](){
        std::stringstream sstr;
        sstr << vertex_shader_preamble();
        sstr << "layout (location = 0) in vec2 aPos;\n";
        sstr << "layout (location = 1) in vec2 aTexCoords;\n";
        sstr << "\n";
        sstr << "out vec2 TexCoords;\n";
        sstr << "\n";
        sstr << "void main()\n";
        sstr << "{\n";
        sstr << "    TexCoords = aTexCoords;\n";
        sstr << "    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n";
        sstr << "}";
        return sstr.str();
    };
    return result();
}

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
        vertices_.init(quad_vertices_, quad_vertices_ + 4 * 6);
        vertices_.wait();
        CHK(glEnableVertexAttribArray(0));
        CHK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr));
        CHK(glEnableVertexAttribArray(1));
        CHK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))));
        CHK(glBindVertexArray(0));
    }
    return va_;
}
