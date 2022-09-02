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
    if (interior_mapping_buffer != (GLuint)-1) {
        WARN(glDeleteBuffers(1, &interior_mapping_buffer));
    }
    if (billboard_id_buffer != (GLuint)-1) {
        WARN(glDeleteBuffers(1, &billboard_id_buffer));
    }
}

void VertexArray::gc_deallocate() {
    if (vertex_array != (GLuint)-1) {
        render_gc_append_to_vertex_arrays(vertex_array);
    }
    if (vertex_buffer != (GLuint)-1) {
        render_gc_append_to_buffers(vertex_buffer);
    }
    if (position_buffer != (GLuint)-1) {
        render_gc_append_to_buffers(position_buffer);
    }
    if (bone_weight_buffer != (GLuint)-1) {
        render_gc_append_to_buffers(bone_weight_buffer);
    }
    if (interior_mapping_buffer != (GLuint)-1) {
        render_gc_append_to_buffers(interior_mapping_buffer);
    }
    if (billboard_id_buffer != (GLuint)-1) {
        render_gc_append_to_buffers(billboard_id_buffer);
    }
}
