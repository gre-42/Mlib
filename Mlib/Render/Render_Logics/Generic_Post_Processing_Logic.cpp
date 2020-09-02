#include "Generic_Post_Processing_Logic.hpp"
#include <Mlib/Render/CHK.hpp>

using namespace Mlib;

const char* GenericPostProcessingLogic::vertex_shader_text =
"#version 330 core\n"
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

GenericPostProcessingLogic::GenericPostProcessingLogic() {
    float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    {
        // screen quad VAO
        CHK(glGenVertexArrays(1, &va_.vertex_array));
        CHK(glGenBuffers(1, &va_.vertex_buffer));
        CHK(glBindVertexArray(va_.vertex_array));
        CHK(glBindBuffer(GL_ARRAY_BUFFER, va_.vertex_buffer));
        CHK(glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW));
        CHK(glEnableVertexAttribArray(0));
        CHK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0));
        CHK(glEnableVertexAttribArray(1));
        CHK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))));
        CHK(glBindVertexArray(0));
    }
}
