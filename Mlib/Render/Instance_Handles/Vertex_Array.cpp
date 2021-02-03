#include "Vertex_Array.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Render_Garbage_Collector.hpp>

using namespace Mlib;

VertexArray::~VertexArray() {
    if (glfwGetCurrentContext() != nullptr) {
        deallocate();
    } else {
        gc_deallocate();
    }
}

void VertexArray::deallocate() {
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

void VertexArray::gc_deallocate() {
    if (vertex_array != (GLuint)-1) {
        gc_vertex_arrays.push_back(vertex_array);
    }
    if (vertex_buffer != (GLuint)-1) {
        gc_buffers.push_back(vertex_buffer);
    }
    if (position_buffer != (GLuint)-1) {
        gc_buffers.push_back(position_buffer);
    }
    if (bone_weight_buffer != (GLuint)-1) {
        gc_buffers.push_back(bone_weight_buffer);
    }
}
