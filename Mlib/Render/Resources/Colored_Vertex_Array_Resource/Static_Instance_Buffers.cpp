#include "Static_Instance_Buffers.hpp"
#include <Mlib/Geometry/Material/Transformation_Mode.hpp>
#include <Mlib/Geometry/Mesh/Transformation_And_Billboard_Id.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

StaticInstanceBuffers::StaticInstanceBuffers(
    TransformationMode transformation_mode,
    std::vector<TransformationAndBillboardId>&& instances_moved_from,
    BillboardId num_billboard_atlas_components,
    const std::string& name)
    : instances_{ std::move(instances_moved_from) }
    , position_yangles_{ instances_ }
    , position_{ instances_ }
    , rotation_quaternion{ instances_ }
    , billboard_ids_{ instances_, num_billboard_atlas_components }
    , num_billboard_atlas_components_{ num_billboard_atlas_components }
    , transformation_mode_{ transformation_mode }
{
    if (instances_.empty()) {
        THROW_OR_ABORT("StaticInstanceBuffers::StaticInstanceBuffers received empty instances \"" + name + '"');
    }
}

StaticInstanceBuffers::~StaticInstanceBuffers() = default;

bool StaticInstanceBuffers::copy_in_progress() const {
    if (transformation_mode_ == TransformationMode::POSITION_YANGLE) {
        if (position_yangles_.copy_in_progress()) {
            return true;
        }
    } else if ((transformation_mode_ == TransformationMode::POSITION) ||
               (transformation_mode_ == TransformationMode::POSITION_LOOKAT) ||
               (transformation_mode_ == TransformationMode::ALL)) {
        if (position_.copy_in_progress()) {
            return true;
        }
    } else {
        THROW_OR_ABORT("Unsupported transformation mode for instances");
    }
    if (transformation_mode_ == TransformationMode::ALL) {
        if (rotation_quaternion.copy_in_progress()) {
            return true;
        }
    }
    if (num_billboard_atlas_components_ != 0) {
        if (billboard_ids_.copy_in_progress()) {
            return true;
        }
    }
    return false;
}

void StaticInstanceBuffers::wait() const {
    if (transformation_mode_ == TransformationMode::POSITION_YANGLE) {
        position_yangles_.wait();
    } else if ((transformation_mode_ == TransformationMode::POSITION) ||
               (transformation_mode_ == TransformationMode::POSITION_LOOKAT) ||
               (transformation_mode_ == TransformationMode::ALL))
    {
        position_.wait();
    } else {
        THROW_OR_ABORT("Unsupported transformation mode for instances");
    }
    if (transformation_mode_ == TransformationMode::ALL) {
        rotation_quaternion.wait();
    }
    if (num_billboard_atlas_components_ != 0) {
        billboard_ids_.wait();
    }
}

void StaticInstanceBuffers::update() {
}

void StaticInstanceBuffers::bind(
    GLuint instance_attribute_index,
    GLuint rotation_quaternion_attribute_index,
    GLuint billboard_ids_attribute_index,
    GLuint texture_layer_attribute_index,
    TaskLocation task_location) const
{
    if (transformation_mode_ == TransformationMode::POSITION_YANGLE) {
        position_yangles_.bind(instance_attribute_index, task_location);
    } else if ((transformation_mode_ == TransformationMode::POSITION) ||
               (transformation_mode_ == TransformationMode::POSITION_LOOKAT) ||
               (transformation_mode_ == TransformationMode::ALL))
    {
        position_.bind(instance_attribute_index, task_location);
    } else {
        THROW_OR_ABORT("Unsupported transformation mode for instances");
    }
    if (transformation_mode_ == TransformationMode::ALL) {
        rotation_quaternion.bind(
            rotation_quaternion_attribute_index,
            task_location);
    }
    if (num_billboard_atlas_components_ != 0) {
        billboard_ids_.bind(billboard_ids_attribute_index, task_location);
    }
}

GLsizei StaticInstanceBuffers::num_instances() const {
    return integral_cast<GLsizei>(instances_.size());
}

bool StaticInstanceBuffers::has_continuous_texture_layer() const {
    return false;
}
