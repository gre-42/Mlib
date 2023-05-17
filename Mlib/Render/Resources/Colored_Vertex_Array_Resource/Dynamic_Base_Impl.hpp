#pragma once
#include "Dynamic_Base.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/CHK.hpp>

namespace Mlib {

template <class tvalue_type>
DynamicBase<tvalue_type>::DynamicBase(GLsizei max_num_instances)
: max_num_instances_{max_num_instances},
  num_instances_{0}
{
    CHK(glGenBuffers(1, &buffer_));
    if (buffer_ == (GLuint)-1) {
        THROW_OR_ABORT("Unsupported buffer index");
    }
    CHK(glBindBuffer(GL_ARRAY_BUFFER, buffer_));
    CHK(glBufferData(GL_ARRAY_BUFFER, integral_cast<GLsizeiptr>(integral_cast<GLsizei>(sizeof(value_type)) * max_num_instances), nullptr, GL_DYNAMIC_DRAW));

    CHK(glBindBuffer(GL_ARRAY_BUFFER, buffer_));
    CHK(instances_ = (value_type*)glMapNamedBuffer(buffer_, GL_READ_WRITE));
    CHK(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

template <class tvalue_type>
DynamicBase<tvalue_type>::~DynamicBase() {
    ABORT(glUnmapNamedBuffer(buffer_));
    ABORT(glDeleteBuffers(1, &buffer_));
}

template <class tvalue_type>
void DynamicBase<tvalue_type>::append(const value_type& v) {
    if (num_instances_ == max_num_instances_) {
        THROW_OR_ABORT("Too many instances");
    }
    instances_[num_instances_++] = v;
}

template <class tvalue_type>
void DynamicBase<tvalue_type>::remove(GLsizei index) {
    if (index >= num_instances_) {
        THROW_OR_ABORT("Billboard index out of bounds");
    }
    if (num_instances_ > 0) {
        instances_[index] = instances_[--num_instances_];
    }
}

template <class tvalue_type>
void DynamicBase<tvalue_type>::bind() const {
    CHK(glBindBuffer(GL_ARRAY_BUFFER, buffer_));
}


}
