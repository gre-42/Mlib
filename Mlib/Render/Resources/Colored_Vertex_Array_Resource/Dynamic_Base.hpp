#pragma once
#include <Mlib/Default_Uninitialized.hpp>
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Render/Any_Gl.hpp>
#include <Mlib/Default_Uninitialized_Vector.hpp>

namespace Mlib {

template <class tvalue_type>
class DynamicBase {
    DynamicBase(const DynamicBase&) = delete;
    DynamicBase& operator = (const DynamicBase&) = delete;
public:
    using value_type = tvalue_type;

    explicit DynamicBase(size_t max_num_instances);
    ~DynamicBase();
    void append(const tvalue_type& v);
    void remove(size_t index);
    void clear();
    tvalue_type& operator [] (size_t index);
    void update();
    void bind() const;
    size_t size() const;    // for debugging purposes only
    size_t capacity() const;
private:
    void allocate();
    void deallocate();
    UVector<value_type> instances_;
    size_t max_num_instances_;
    size_t num_instances_;
    mutable GLuint buffer_;
    DeallocationToken deallocation_token_;
};

}

#include "Dynamic_Base_Impl.hpp"
