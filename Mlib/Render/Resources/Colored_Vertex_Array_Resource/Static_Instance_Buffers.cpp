#include "Static_Instance_Buffers.hpp"
#include <Mlib/Geometry/Mesh/Transformation_And_Billboard_Id.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Scene_Graph/Transformation_Mode.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

StaticInstanceBuffers::StaticInstanceBuffers(
    TransformationMode transformation_mode,
    std::vector<TransformationAndBillboardId>&& instances,
    uint32_t num_billboard_atlas_components,
    const std::string& name)
: instances_{std::move(instances)},
  position_yangles_{instances_},
  position_{instances_},
  billboard_ids_{instances_, num_billboard_atlas_components},
  num_billboard_atlas_components_{num_billboard_atlas_components},
  transformation_mode_{transformation_mode}
{
    if (instances_.empty()) {
        THROW_OR_ABORT("StaticInstanceBuffers::StaticInstanceBuffers received empty instances \"" + name + '"');
    }
}

StaticInstanceBuffers::~StaticInstanceBuffers() = default;

void StaticInstanceBuffers::update()
{}

void StaticInstanceBuffers::bind(
    GLuint instance_attribute_index,
    GLuint billboard_ids_attribute_index) const
{
    if (transformation_mode_ == TransformationMode::POSITION_YANGLE) {
        position_yangles_.bind(instance_attribute_index);
    } else if ((transformation_mode_ == TransformationMode::POSITION) ||
               (transformation_mode_ == TransformationMode::POSITION_LOOKAT))
    {
        position_.bind(instance_attribute_index);
    } else {
        THROW_OR_ABORT("Unsupported transformation mode for instances");
    }
    if (num_billboard_atlas_components_ != 0) {
        billboard_ids_.bind(billboard_ids_attribute_index);
    }
}

size_t StaticInstanceBuffers::tmp_num_instances() const {
    return instances_.size();
}

GLsizei StaticInstanceBuffers::num_instances() const {
    return integral_cast<GLsizei>(instances_.size());
}
