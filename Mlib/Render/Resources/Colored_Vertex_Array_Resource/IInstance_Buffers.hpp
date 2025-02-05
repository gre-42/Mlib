#pragma once
#include <Mlib/Render/Any_Gl.hpp>
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
enum class TaskLocation;

class IInstanceBuffers {
public:
    virtual ~IInstanceBuffers() = default;
    virtual bool copy_in_progress() const = 0;
    virtual void wait() const = 0;
    virtual void bind(
        GLuint instance_attribute_index,
        GLuint rotation_quaternion_attribute_index,
        GLuint billboard_ids_attribute_index,
        GLuint texture_layer_attribute_index,
        TaskLocation task_location) const = 0;
    virtual GLsizei num_instances() const = 0;
    virtual bool has_continuous_texture_layer() const = 0;
};

}
