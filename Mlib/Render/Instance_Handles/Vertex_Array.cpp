#include "Vertex_Array.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Context_Query.hpp>
#include <Mlib/Render/Deallocate/Deallocation_Mode.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Deallocate/Render_Garbage_Collector.hpp>
#include <Mlib/Render/Instance_Handles/IArray_Buffer.hpp>

using namespace Mlib;

VertexArray::VertexArray(
    IArrayBuffer& vertex_buffer,
    IArrayBuffer& bone_weight_buffer,
    IArrayBuffer& texture_layer_buffer,
    IArrayBuffer& interior_mapping_buffer)
    : vertex_buffer{ vertex_buffer }
    , bone_weight_buffer{ bone_weight_buffer }
    , texture_layer_buffer{ texture_layer_buffer }
    , interior_mapping_buffer{ interior_mapping_buffer }
    , vertex_array_ { (GLuint)-1 }
    , deallocation_token_{ render_deallocator.insert([this]() {deallocate(DeallocationMode::DIRECT); }) }
{}

VertexArray::~VertexArray() {
    if (ContextQuery::is_initialized()) {
        deallocate(DeallocationMode::DIRECT);
    } else {
        deallocate(DeallocationMode::GARBAGE_COLLECTION);
    }
}

bool VertexArray::initialized() const {
    return (vertex_array_ != (GLuint)-1);
}

void VertexArray::initialize() {
    if (initialized()) {
        THROW_OR_ABORT("Vertex-array already initialized");
    }
    CHK(glGenVertexArrays(1, &vertex_array_));
    CHK(glBindVertexArray(vertex_array_));
    if (!initialized()) {
        verbose_abort("Unexpected vertex-array index");
    }
}

bool VertexArray::copy_in_progress() const {
    return
        vertex_buffer.copy_in_progress() ||
        bone_weight_buffer.copy_in_progress() ||
        texture_layer_buffer.copy_in_progress() ||
        interior_mapping_buffer.copy_in_progress();
}

void VertexArray::update() {
    vertex_buffer.update();
    bone_weight_buffer.update();
    texture_layer_buffer.update();
    interior_mapping_buffer.update();
}

void VertexArray::wait() const {
    vertex_buffer.wait();
    bone_weight_buffer.wait();
    texture_layer_buffer.wait();
    interior_mapping_buffer.wait();
}

void VertexArray::bind() const {
    if (!initialized()) {
        THROW_OR_ABORT("Vertex-array not initialized");
    }
    wait();
    CHK(glBindVertexArray(vertex_array_));
}

void VertexArray::deallocate(DeallocationMode mode) {
    if (initialized()) {
        if (mode == DeallocationMode::DIRECT) {
            ABORT(glDeleteVertexArrays(1, &vertex_array_));
        } else if (mode == DeallocationMode::GARBAGE_COLLECTION) {
            render_gc_append_to_vertex_arrays(vertex_array_);
        } else {
            verbose_abort("Unknown deallocation mode");
        }
        vertex_array_ = (GLuint)-1;
    }
    // The buffers have to deallocate themselves.
    // vertex_buffer.deallocate(mode);
    // bone_weight_buffer.deallocate(mode);
    // texture_layer_buffer.deallocate(mode);
    // interior_mapping_buffer.deallocate(mode);
}
