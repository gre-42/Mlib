#pragma once
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace Mlib {

struct VertexArray {
    VertexArray() = default;
    VertexArray(const VertexArray&) = delete;
    VertexArray& operator = (const VertexArray&) = delete;
    ~VertexArray();
    GLuint vertex_array = (GLuint)-1;
    GLuint vertex_buffer = (GLuint)-1;
    GLuint position_buffer = (GLuint)-1;
    GLuint bone_weight_buffer = (GLuint)-1;
    void free();
};

}
