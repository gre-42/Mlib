#pragma once
#include <Mlib/Render/Any_Gl.hpp>
#include <vector>

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
    void modify(GLsizei index, const value_type& v);
    void update();
    void bind() const;
private:
    void allocate();
    std::vector<value_type> instances_;
    GLsizei max_num_instances_;
    GLsizei num_instances_;
    mutable GLuint buffer_;
};

}

#include "Dynamic_Base_Impl.hpp"
