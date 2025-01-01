#pragma once
#include "Dynamic_Base.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Context_Query.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Deallocate/Render_Garbage_Collector.hpp>
#include <Mlib/Render/Deallocate/Render_Try_Delete.hpp>

namespace Mlib {

template <class tvalue_type>
DynamicBase<tvalue_type>::DynamicBase(size_t max_num_instances)
    : instances_(max_num_instances)
    , max_num_instances_{max_num_instances}
    , num_instances_{ 0 }
    , buffer_{ (GLuint)-1 }
    , deallocation_token_{ render_deallocator.insert([this]() { deallocate(); }) }
{
    if (ContextQuery::is_initialized()) {
        allocate();
    }
}

template <class tvalue_type>
void DynamicBase<tvalue_type>::allocate() {
    CHK(glGenBuffers(1, &buffer_));
    if (buffer_ == (GLuint)-1) {
        THROW_OR_ABORT("Unsupported buffer index");
    }
    CHK(glBindBuffer(GL_ARRAY_BUFFER, buffer_));
    CHK(glBufferData(GL_ARRAY_BUFFER, integral_cast<GLsizeiptr>(sizeof(value_type) * max_num_instances_), nullptr, GL_DYNAMIC_DRAW));
}

template <class tvalue_type>
DynamicBase<tvalue_type>::~DynamicBase() {
    deallocate();
}

template <class tvalue_type>
void DynamicBase<tvalue_type>::deallocate() {
    if (buffer_ != (GLuint)-1) {
        try_delete_buffer(buffer_);
    }
}

template <class tvalue_type>
void DynamicBase<tvalue_type>::append(const tvalue_type& v) {
    if (num_instances_ == max_num_instances_) {
        THROW_OR_ABORT("Too many instances");
    }
    instances_[num_instances_++] = v;
}

template <class tvalue_type>
void DynamicBase<tvalue_type>::remove(size_t index) {
    if (index >= num_instances_) {
        THROW_OR_ABORT("Billboard index out of bounds");
    }
    --num_instances_;
    if (num_instances_ != 0) {
        instances_[index] = instances_[num_instances_];
    }
}

template <class tvalue_type>
void DynamicBase<tvalue_type>::clear() {
    num_instances_ = 0;
}

template <class tvalue_type>
tvalue_type& DynamicBase<tvalue_type>::operator [] (size_t index) {
    if (index >= num_instances_) {
        THROW_OR_ABORT("Billboard index out of bounds");
    }
    return instances_[index];
}

template <class tvalue_type>
void DynamicBase<tvalue_type>::update() {
    if (buffer_ == (GLuint)-1) {
        THROW_OR_ABORT("Buffer update before bind or allocation in ctor");
    }
    if (num_instances_ == 0) {
        THROW_OR_ABORT("Number of instances is zero");
    }
    CHK(glBindBuffer(GL_ARRAY_BUFFER, buffer_));
    // // CHK(auto* instances_gpu = (value_type*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));
    // CHK(auto* instances_gpu = (value_type*)glMapBufferRange(GL_ARRAY_BUFFER, 0, integral_cast<GLsizeiptr>(num_instances_ * sizeof(value_type)), GL_MAP_WRITE_BIT));
    // std::copy(
    //     instances_.data(),
    //     instances_.data() + num_instances_,
    //     instances_gpu);
    // CHK(glUnmapBuffer(GL_ARRAY_BUFFER));
    CHK(glBufferSubData(GL_ARRAY_BUFFER, 0, integral_cast<GLsizeiptr>(num_instances_ * sizeof(value_type)), instances_.data()));
    CHK(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

template <class tvalue_type>
void DynamicBase<tvalue_type>::bind() const {
    if (buffer_ == (GLuint)-1) {
        const_cast<DynamicBase<tvalue_type>*>(this)->allocate();
    }
    CHK(glBindBuffer(GL_ARRAY_BUFFER, buffer_));
}

template <class tvalue_type>
size_t DynamicBase<tvalue_type>::size() const {
    return num_instances_;
}

template <class tvalue_type>
size_t DynamicBase<tvalue_type>::capacity() const {
    return max_num_instances_;
}

}
