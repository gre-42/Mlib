#include "Static_Instance_Buffers.hpp"
#include <Mlib/Geometry/Mesh/Transformation_And_Billboard_Id.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

StaticInstanceBuffers::StaticInstanceBuffers(
    std::vector<TransformationAndBillboardId>&& instances,
    uint32_t num_billboard_atlas_components,
    const std::string& name)
: instances_{std::move(instances)},
  position_yangles_{instances_},
  position_{instances_},
  billboard_atlas_instances_{instances_, num_billboard_atlas_components}
{
    if (instances_.empty()) {
        THROW_OR_ABORT("StaticInstanceBuffers::StaticInstanceBuffers received empty instances \"" + name + '"');
    }
}

StaticInstanceBuffers::~StaticInstanceBuffers() = default;

void StaticInstanceBuffers::bind_position_yangles(GLuint attribute_index) const {
    position_yangles_.bind(attribute_index);
}

void StaticInstanceBuffers::bind_position(GLuint attribute_index) const {
    position_.bind(attribute_index);
}

void StaticInstanceBuffers::bind_billboard_atlas_instances(GLuint attribute_index) const
{
    billboard_atlas_instances_.bind(attribute_index);
}

size_t StaticInstanceBuffers::num_instances() const {
    return instances_.size();
}
