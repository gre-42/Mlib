#include "Static_Billboard_Ids.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Mesh/Transformation_And_Billboard_Id.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Render/CHK.hpp>

using namespace Mlib;

StaticBillboardIds::StaticBillboardIds(
    const std::vector<TransformationAndBillboardId>& instances,
    BillboardId num_billboard_atlas_components)
    : instances_{ instances }
    , num_billboard_atlas_components_{ num_billboard_atlas_components }
{
    if (num_billboard_atlas_components_ == 0) {
        return;
    }
    billboard_ids_.reserve(instances_.size());
    for (const TransformationAndBillboardId& m : instances_) {
        if (m.billboard_id >= num_billboard_atlas_components_) {
            THROW_OR_ABORT("Billboard ID too large");
        }
        billboard_ids_.push_back(m.billboard_id);
    }
}

StaticBillboardIds::~StaticBillboardIds() = default;

bool StaticBillboardIds::copy_in_progress() const {
    if (num_billboard_atlas_components_ == 0) {
        return false;
    }
    return buffer_.copy_in_progress();
}

void StaticBillboardIds::wait() const {
    buffer_.wait();
}

void StaticBillboardIds::bind(
    GLuint attribute_index,
    TaskLocation task_location) const
{
    if (num_billboard_atlas_components_ == 0) {
        THROW_OR_ABORT("Attempt to bind zero billboard-IDs");
    }
    buffer_.set(billboard_ids_, task_location);

    CHK(glEnableVertexAttribArray(attribute_index));
    static_assert(std::is_same_v<BillboardId, uint16_t>);
    CHK(glVertexAttribIPointer(attribute_index, 1, GL_UNSIGNED_SHORT, sizeof(BillboardId), nullptr));
    CHK(glVertexAttribDivisor(attribute_index, 1));
}
