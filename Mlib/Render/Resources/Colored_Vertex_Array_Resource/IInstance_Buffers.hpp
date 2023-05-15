#pragma once
#include <Mlib/Render/Any_Gl.hpp>
#include <cstddef>

namespace Mlib {

class IInstanceBuffers {
public:
    virtual ~IInstanceBuffers() = default;
    virtual void bind_position_yangles(GLuint attribute_index) const = 0;
    virtual void bind_position(GLuint attribute_index) const = 0;
    virtual void bind_billboard_atlas_instances(
        GLuint attribute_index,
        uint32_t num_billboard_atlas_instances) const = 0;
    virtual size_t num_instances() const = 0;
};

}
