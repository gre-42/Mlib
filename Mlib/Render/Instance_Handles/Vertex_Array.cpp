#include "Vertex_Array.hpp"
#include <Mlib/Render/CHK.hpp>

using namespace Mlib;

VertexArray::~VertexArray() {
    if (glfwGetCurrentContext() != nullptr) {
        free();
    }
}

void VertexArray::free() {
    if (vertex_array != (GLuint)-1) {
        WARN(glDeleteVertexArrays(1, &vertex_array));
    }
    if (vertex_buffer != (GLuint)-1) {
        WARN(glDeleteBuffers(1, &vertex_buffer));
    }
    if (position_buffer != (GLuint)-1) {
        WARN(glDeleteBuffers(1, &position_buffer));
    }
    if (bone_weight_buffer != (GLuint)-1) {
        WARN(glDeleteBuffers(1, &bone_weight_buffer));
    }
}
