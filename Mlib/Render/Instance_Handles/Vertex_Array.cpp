#include "Vertex_Array.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Context_Query.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Deallocate/Render_Garbage_Collector.hpp>

using namespace Mlib;

VertexArray::VertexArray()
: deallocation_token_{render_deallocator.insert([this](){deallocate();})}
{}

VertexArray::~VertexArray() {
    if (ContextQuery::is_initialized()) {
        deallocate();
    } else {
        gc_deallocate();
    }
}

void VertexArray::deallocate() {
    if (vertex_array != (GLuint)-1) {
        WARN(glDeleteVertexArrays(1, &vertex_array));
        vertex_array = (GLuint)-1;
    }
    if (vertex_buffer != (GLuint)-1) {
        WARN(glDeleteBuffers(1, &vertex_buffer));
        vertex_buffer = (GLuint)-1;
    }
    if (bone_weight_buffer != (GLuint)-1) {
        WARN(glDeleteBuffers(1, &bone_weight_buffer));
        bone_weight_buffer = (GLuint)-1;
    }
    if (interior_mapping_buffer != (GLuint)-1) {
        WARN(glDeleteBuffers(1, &interior_mapping_buffer));
        interior_mapping_buffer = (GLuint)-1;
    }
}

void VertexArray::gc_deallocate() {
    if (vertex_array != (GLuint)-1) {
        render_gc_append_to_vertex_arrays(vertex_array);
    }
    if (vertex_buffer != (GLuint)-1) {
        render_gc_append_to_buffers(vertex_buffer);
    }
    if (bone_weight_buffer != (GLuint)-1) {
        render_gc_append_to_buffers(bone_weight_buffer);
    }
    if (interior_mapping_buffer != (GLuint)-1) {
        render_gc_append_to_buffers(interior_mapping_buffer);
    }
}
