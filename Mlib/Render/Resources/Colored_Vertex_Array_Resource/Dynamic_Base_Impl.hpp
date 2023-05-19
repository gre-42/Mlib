#pragma once
#include "Dynamic_Base.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Context_Query.hpp>
#include <Mlib/Render/Deallocate/Render_Garbage_Collector.hpp>

namespace Mlib {

template <class tvalue_type>
DynamicBase<tvalue_type>::DynamicBase(size_t max_num_instances)
: instances_(max_num_instances),
  max_num_instances_{max_num_instances},
  num_instances_{0},
  buffer_{(GLuint)-1}
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
    if (buffer_ != (GLuint)-1) {
        if (ContextQuery::is_initialized()) {
            ABORT(glDeleteBuffers(1, &buffer_));
        } else {
            render_gc_append_to_buffers(buffer_);
        }
    }
}

template <class tvalue_type>
void DynamicBase<tvalue_type>::append(const value_type& v) {
    if (num_instances_ == max_num_instances_) {
        THROW_OR_ABORT("Too many instances");
    }
    instances_[integral_cast<size_t>(num_instances_++)] = v;
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
void DynamicBase<tvalue_type>::modify(size_t index, const value_type& v) {
    if (index >= num_instances_) {
        THROW_OR_ABORT("Billboard index out of bounds");
    }
    instances_[index] = v;
}

template <class tvalue_type>
void DynamicBase<tvalue_type>::update() {
    if (buffer_ == (GLuint)-1) {
        THROW_OR_ABORT("Buffer update before bind or allocation in ctor");
    }
    CHK(glBindBuffer(GL_ARRAY_BUFFER, buffer_));
    CHK(value_type* instances_gpu = (value_type*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));
    std::copy(
        instances_.data(),
        instances_.data() + num_instances_,
        instances_gpu);
    CHK(glUnmapBuffer(GL_ARRAY_BUFFER));
    CHK(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

template <class tvalue_type>
void DynamicBase<tvalue_type>::bind() const {
    if (buffer_ == (GLuint)-1) {
        const_cast<DynamicBase<tvalue_type>*>(this)->allocate();
    }
    CHK(glBindBuffer(GL_ARRAY_BUFFER, buffer_));
}

}
