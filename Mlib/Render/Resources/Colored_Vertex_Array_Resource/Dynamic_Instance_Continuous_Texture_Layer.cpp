#include "Dynamic_Instance_Continuous_Texture_Layer.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Mesh/Transformation_And_Billboard_Id.hpp>
#include <Mlib/Render/CHK.hpp>

using namespace Mlib;

DynamicInstanceContinuousTextureLayer::DynamicInstanceContinuousTextureLayer(size_t max_num_instances)
    : DynamicBase<value_type>{ max_num_instances }
{}

DynamicInstanceContinuousTextureLayer::~DynamicInstanceContinuousTextureLayer() = default;

void DynamicInstanceContinuousTextureLayer::bind(GLuint attribute_index) const {
    DynamicBase<value_type>::bind();

    CHK(glEnableVertexAttribArray(attribute_index));
    CHK(glVertexAttribPointer(attribute_index, 1, GL_FLOAT, GL_FALSE, sizeof(value_type), nullptr));
    CHK(glVertexAttribDivisor(attribute_index, 1));
}
