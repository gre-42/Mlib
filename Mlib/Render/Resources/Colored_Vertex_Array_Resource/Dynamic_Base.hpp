#pragma once
#include <Mlib/Render/Any_Gl.hpp>
#include <vector>
#include <Mlib/Memory/Deallocation_Token.hpp>

namespace Mlib {

template <class tvalue_type>
class DynamicBase {
    DynamicBase(const DynamicBase&) = delete;
    DynamicBase& operator = (const DynamicBase&) = delete;
public:
    using value_type = tvalue_type;

    explicit DynamicBase(size_t max_num_instances);
    ~DynamicBase();
    void append(const value_type& v);
    void remove(size_t index);
    void modify(size_t index, const value_type& v);
    void update();
    void bind() const;
private:
    void allocate();
    void deallocate();
    std::vector<value_type> instances_;
    size_t max_num_instances_;
    size_t num_instances_;
    mutable GLuint buffer_;
    DeallocationToken deallocation_token_;
};

}

#include "Dynamic_Base_Impl.hpp"
