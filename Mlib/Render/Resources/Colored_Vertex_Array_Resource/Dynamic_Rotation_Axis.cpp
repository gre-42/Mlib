#include "Dynamic_Rotation_Axis.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Mesh/Transformation_And_Billboard_Id.hpp>
#include <Mlib/Render/CHK.hpp>

using namespace Mlib;

DynamicRotationAxis::DynamicRotationAxis(
    size_t max_num_instances,
    size_t axis)
    : DynamicBase<value_type>{ max_num_instances }
    , axis_{ axis }
{}

DynamicRotationAxis::~DynamicRotationAxis() = default;

void DynamicRotationAxis::append(const TransformationAndBillboardId& m) {
    DynamicBase<value_type>::append(m.transformation_matrix.R().column(axis_));
}

void DynamicRotationAxis::bind(GLuint attribute_index) const {
    DynamicBase<value_type>::bind();

    CHK(glEnableVertexAttribArray(attribute_index));
    CHK(glVertexAttribPointer(attribute_index, 3, GL_FLOAT, GL_FALSE, sizeof(value_type), nullptr));
    CHK(glVertexAttribDivisor(attribute_index, 1));
}
