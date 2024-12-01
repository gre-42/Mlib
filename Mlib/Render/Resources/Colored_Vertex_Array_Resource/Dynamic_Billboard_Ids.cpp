#include "Dynamic_Billboard_Ids.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Mesh/Transformation_And_Billboard_Id.hpp>
#include <Mlib/Render/CHK.hpp>

using namespace Mlib;

DynamicBillboardIds::DynamicBillboardIds(
    size_t max_num_instances,
    BillboardId num_billboard_atlas_components)
    : DynamicBase<value_type>{ max_num_instances }
    , num_billboard_atlas_components_{ num_billboard_atlas_components }
{}

DynamicBillboardIds::~DynamicBillboardIds() = default;

void DynamicBillboardIds::append(const TransformationAndBillboardId& m) {
    if (m.billboard_id >= num_billboard_atlas_components_) {
        THROW_OR_ABORT("Billboard ID too large");
    }
    DynamicBase<value_type>::append(m.billboard_id);
}

void DynamicBillboardIds::bind(GLuint attribute_index) const {
    DynamicBase<value_type>::bind();

    CHK(glEnableVertexAttribArray(attribute_index));
    static_assert(std::is_same_v<BillboardId, uint16_t>);
    CHK(glVertexAttribIPointer(attribute_index, 1, GL_UNSIGNED_SHORT, sizeof(value_type), nullptr));
    CHK(glVertexAttribDivisor(attribute_index, 1));
}
