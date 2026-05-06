#include "Vertex_Array.hpp"
#include <Mlib/OpenGL/CHK.hpp>
#include <Mlib/OpenGL/Context_Query.hpp>
#include <Mlib/OpenGL/Deallocate/Deallocation_Mode.hpp>
#include <Mlib/OpenGL/Deallocate/Render_Deallocator.hpp>
#include <Mlib/OpenGL/Deallocate/Render_Garbage_Collector.hpp>
#include <Mlib/OpenGL/Instance_Handles/IArray_Buffer.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Instance_Buffers.hpp>
#include <iostream>

using namespace Mlib;

VertexArray::VertexArray()
    : instance_buffer_{ nullptr }
    , vertex_array_{ (GLuint)-1 }
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
    if (initialized()) {
        throw std::runtime_error("Vertex array already initialized");
    }
    array_buffers_.push_back(&array_buffer);
}

void VertexArray::set_instance_buffer(IGpuInstanceBuffers& instance_buffer) {
    if (initialized()) {
        throw std::runtime_error("Vertex array already initialized");
    }
    if (instance_buffer_ != nullptr) {
        throw std::runtime_error("Instance buffer already set");
    }
    instance_buffer_ = &instance_buffer;
}

bool VertexArray::initialized() const {
    return (vertex_array_ != (GLuint)-1);
}

void VertexArray::initialize() {
    if (initialized()) {
        throw std::runtime_error("Vertex-array already initialized");
    }
    CHK(glGenVertexArrays(1, &vertex_array_));
    CHK(glBindVertexArray(vertex_array_));
    if (!initialized()) {
        throw std::runtime_error("Unexpected vertex-array index");
    }
}

bool VertexArray::copy_in_progress() const {
    if (!initialized()) {
        throw std::runtime_error("VertexArray::copy_in_progress on an uninitialized array");
    }
    for (const auto& a : array_buffers_) {
        if (a->copy_in_progress()) {
            return true;
        }
    }
    if ((instance_buffer_ != nullptr) && instance_buffer_->copy_in_progress()) {
        return true;
    }
    return false;
}

void VertexArray::update() {
    if (!initialized()) {
        throw std::runtime_error("VertexArray::update on an uninitialized array");
    }
    for (auto& a : array_buffers_) {
        a->update();
    }
}

void VertexArray::wait() const {
    if (!initialized()) {
        throw std::runtime_error("VertexArray::wait on an uninitialized array");
    }
    for (const auto& a : array_buffers_) {
        a->wait();
    }
    if (instance_buffer_ != nullptr) {
        instance_buffer_->wait();
    }
}

void VertexArray::bind() const {
    if (!initialized()) {
        throw std::runtime_error("VertexArray::bind on an uninitialized array");
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
    array_buffers_.clear();
    instance_buffer_ = nullptr;
    // The buffers have to deallocate themselves.
    // vertex_buffer.deallocate(mode);
    // bone_weight_buffer.deallocate(mode);
    // texture_layer_buffer.deallocate(mode);
    // interior_mapping_buffer.deallocate(mode);
}

void VertexArray::print_stats(std::ostream& ostr) const {
    ostr << "VertexArray\n";
    if (!initialized()) {
        ostr << "  ()\n";
        return;
    }
    ostr << "  #array_buffers: " << array_buffers_.size() << '\n';
    ostr << "  instance buffer: " << (int)(instance_buffer_ != nullptr) << '\n';
}
