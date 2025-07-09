#include "Diff_Vertex_Shader.hpp"
#include <Mlib/Render/Shader_Version_3_0.hpp>
#include <sstream>

using namespace Mlib;

std::string Mlib::diff_vertex_shader(int texture_width, int texture_height) {
    std::stringstream vs;
    vs << SHADER_VER;
    vs << "layout (location = 0) in vec2 aPos;" << std::endl;
    vs << "layout (location = 1) in vec2 aTexCoords;" << std::endl;
    vs << std::endl;
    vs << "out vec2 TexCoords0;" << std::endl;
    for (int dim = 0; dim < 2; ++dim) {
        for (int h = 0; h < 2; ++h) {
            vs << "out vec2 TexCoords" << dim << h << ';' << std::endl;
        }
    }
    vs << std::endl;
    vs << "void main()" << std::endl;
    vs << "{" << std::endl;
    vs << "    TexCoords0 = aTexCoords;" << std::endl;
    for (int h = 0; h < 2; ++h) {
        vs << "    TexCoords0" << h << " = aTexCoords + vec2(" <<
            float(h * 2 - 1) / (float)texture_width << ", 0.0);" << std::endl;
        vs << "    TexCoords1" << h << " = aTexCoords + vec2(0.0, " <<
            float(h * 2 - 1) / (float)texture_height << ");" << std::endl;
    }
    vs << "    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);" << std::endl;
    vs << "}" << std::endl;
    return vs.str();
}
