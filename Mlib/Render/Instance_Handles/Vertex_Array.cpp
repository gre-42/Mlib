#include "Vertex_Array.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Context_Query.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Deallocate/Render_Garbage_Collector.hpp>

using namespace Mlib;

VertexArray::VertexArray()
: vertex_array_{(GLuint)-1},
  deallocation_token_{render_deallocator.insert([this](){deallocate();})}
{}

VertexArray::~VertexArray() {
    if (ContextQuery::is_initialized()) {
        deallocate();
    } else {
        gc_deallocate();
    }
}

bool VertexArray::initialized() const {
    return (vertex_array_ != (GLuint)-1);
}

void VertexArray::initialize() {
    CHK(glGenVertexArrays(1, &vertex_array_));
    CHK(glBindVertexArray(vertex_array_));
    if (vertex_array_ == (GLuint)-1) {
        THROW_OR_ABORT("Unexpected vertex-array index");
    }
}

bool VertexArray::copy_in_progress() const {
    return
        vertex_buffer.copy_in_progress() ||
        bone_weight_buffer.copy_in_progress() ||
        texture_layer_buffer.copy_in_progress() ||
        interior_mapping_buffer.copy_in_progress();
}

GLuint VertexArray::vertex_array() const {
    if (!initialized()) {
        THROW_OR_ABORT("Vertex-array not initialized");
    }
    wait();
    return vertex_array_;
}

void VertexArray::wait() const {
    vertex_buffer.wait();
    bone_weight_buffer.wait();
    texture_layer_buffer.wait();
    interior_mapping_buffer.wait();
}

void VertexArray::deallocate() {
    if (vertex_array_ != (GLuint)-1) {
        ABORT(glDeleteVertexArrays(1, &vertex_array_));
        vertex_array_ = (GLuint)-1;
    }
    vertex_buffer.deallocate();
    bone_weight_buffer.deallocate();
    texture_layer_buffer.deallocate();
    interior_mapping_buffer.deallocate();
}

void VertexArray::gc_deallocate() {
    if (vertex_array_ != (GLuint)-1) {
        render_gc_append_to_vertex_arrays(vertex_array_);
    }
    vertex_buffer.gc_deallocate();
    bone_weight_buffer.gc_deallocate();
    texture_layer_buffer.gc_deallocate();
    interior_mapping_buffer.gc_deallocate();
}
