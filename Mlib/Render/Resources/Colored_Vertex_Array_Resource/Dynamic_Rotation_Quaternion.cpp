#include "Dynamic_Rotation_Quaternion.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Mesh/Transformation_And_Billboard_Id.hpp>
#include <Mlib/Math/Transformation/Quaternion.hpp>
#include <Mlib/Render/CHK.hpp>

using namespace Mlib;

DynamicRotationQuaternion::DynamicRotationQuaternion(size_t max_num_instances)
    : DynamicBase<value_type>{ max_num_instances }
{}

DynamicRotationQuaternion::~DynamicRotationQuaternion() = default;

void DynamicRotationQuaternion::append(const TransformationAndBillboardId& m) {
    DynamicBase<value_type>::append(Quaternion<float>{ m.transformation_matrix.R });
}

void DynamicRotationQuaternion::bind(GLuint attribute_index) const {
    DynamicBase<value_type>::bind();

    CHK(glEnableVertexAttribArray(attribute_index));
    CHK(glVertexAttribPointer(attribute_index, 4, GL_FLOAT, GL_FALSE, sizeof(value_type), nullptr));
    CHK(glVertexAttribDivisor(attribute_index, 1));
}
