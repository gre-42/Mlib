#include "Dynamic_Position_YAngles.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Mesh/Transformation_And_Billboard_Id.hpp>
#include <Mlib/Render/CHK.hpp>

using namespace Mlib;

DynamicPositionYAngles::DynamicPositionYAngles(size_t max_num_instances)
: DynamicBase<value_type>{max_num_instances}
{}

DynamicPositionYAngles::~DynamicPositionYAngles() = default;

void DynamicPositionYAngles::append(const TransformationAndBillboardId& m) {
    DynamicBase<value_type>::append(
        {
            m.transformation_matrix.t(0),
            m.transformation_matrix.t(1),
            m.transformation_matrix.t(2),
            std::atan2(-m.transformation_matrix.R(2, 0), m.transformation_matrix.R(0, 0))
        });
}

void DynamicPositionYAngles::bind(GLuint attribute_index) const {
    DynamicBase<value_type>::bind();

    CHK(glEnableVertexAttribArray(attribute_index));
    CHK(glVertexAttribPointer(attribute_index, 4, GL_FLOAT, GL_FALSE, sizeof(value_type), nullptr));
    CHK(glVertexAttribDivisor(attribute_index, 1));
}
