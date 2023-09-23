#pragma once
#include <Mlib/Render/Any_Gl.hpp>
#include <cstddef>

namespace Mlib {

class IInstanceBuffers {
public:
    virtual ~IInstanceBuffers() = default;
    virtual bool copy_in_progress() const = 0;
    virtual void wait() const = 0;
    virtual void update() = 0;
    virtual void bind(
        GLuint instance_attribute_index,
        GLuint billboard_ids_attribute_index) const = 0;
    virtual size_t tmp_num_instances() const = 0;
    virtual GLsizei num_instances() const = 0;
};

}
