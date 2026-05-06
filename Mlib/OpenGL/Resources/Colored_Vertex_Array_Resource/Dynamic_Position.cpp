#include "Dynamic_Position.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/OpenGL/CHK.hpp>
#include <Mlib/Scene_Graph/Instances/Transformation_And_Billboard_Id.hpp>

using namespace Mlib;

DynamicPosition::DynamicPosition(size_t max_num_instances)
    : DynamicBase<value_type>{ max_num_instances }
{}

DynamicPosition::~DynamicPosition() = default;

void DynamicPosition::append(const TransformationAndBillboardId& m) {
    DynamicBase<value_type>::append(m.transformation_matrix.t);
}

void DynamicPosition::bind(GLuint attribute_index) const {
    DynamicBase<value_type>::bind();

    CHK(glEnableVertexAttribArray(attribute_index));
    CHK(glVertexAttribPointer(attribute_index, 3, GL_FLOAT, GL_FALSE, sizeof(value_type), nullptr));
    CHK(glVertexAttribDivisor(attribute_index, 1));
}
