#pragma once
#include <Mlib/Render/Any_Gl.hpp>

namespace Mlib {

template <class tvalue_type>
class DynamicBase {
    DynamicBase(const DynamicBase&) = delete;
    DynamicBase& operator = (const DynamicBase&) = delete;
public:
    using value_type = tvalue_type;

    explicit DynamicBase(GLsizei max_num_instances);
    ~DynamicBase();
    void append(const value_type& v);
    void remove(GLsizei index);
    void bind() const;
private:
    value_type* instances_;
    GLsizei max_num_instances_;
    GLsizei num_instances_;
    mutable GLuint buffer_;
};

}

#include "Dynamic_Base_Impl.hpp"
