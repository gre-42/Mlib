#include "Vertex_Array.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Context_Query.hpp>
#include <Mlib/Render/Deallocate/Deallocation_Mode.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Deallocate/Render_Garbage_Collector.hpp>
#include <Mlib/Render/Instance_Handles/IArray_Buffer.hpp>

using namespace Mlib;

VertexArray::VertexArray()
    : vertex_array_ { (GLuint)-1 }
    , deallocation_token_{ render_deallocator.insert([this]() {deallocate(DeallocationMode::DIRECT); }) }
{}

VertexArray::~VertexArray() {
    if (ContextQuery::is_initialized()) {
        deallocate(DeallocationMode::DIRECT);
    } else {
        deallocate(DeallocationMode::GARBAGE_COLLECTION);
    }
}

void VertexArray::add_array_buffer(IArrayBuffer& array_buffer) {
    array_buffers_.push_back(&array_buffer);
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
    for (const auto& a : array_buffers_) {
        if (a->copy_in_progress()) {
            return true;
        }
    }
    return false;
}

void VertexArray::update() {
    for (auto& a : array_buffers_) {
        a->update();
    }
}

void VertexArray::wait() const {
    for (const auto& a : array_buffers_) {
        a->wait();
    }
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
