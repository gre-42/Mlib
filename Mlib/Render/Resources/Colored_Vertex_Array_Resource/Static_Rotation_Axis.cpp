#include "Static_Rotation_Axis.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Mesh/Transformation_And_Billboard_Id.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Render/CHK.hpp>

using namespace Mlib;

StaticRotationAxis::StaticRotationAxis(
    const std::vector<TransformationAndBillboardId>& instances,
    size_t axis)
    : instances_{ instances }
{
    axes_.reserve(instances_.size());
    for (const TransformationAndBillboardId& m : instances_) {
        axes_.push_back(m.transformation_matrix.R().column(axis));
    }
}

StaticRotationAxis::~StaticRotationAxis() = default;

bool StaticRotationAxis::copy_in_progress() const {
    return buffer_.copy_in_progress();
}

void StaticRotationAxis::wait() const {
    buffer_.wait();
}

void StaticRotationAxis::bind(GLuint attribute_index) const {
    buffer_.set(axes_);

    CHK(glEnableVertexAttribArray(attribute_index));
    CHK(glVertexAttribPointer(attribute_index, 3, GL_FLOAT, GL_FALSE, sizeof(RotationAxis), nullptr));
    CHK(glVertexAttribDivisor(attribute_index, 1));
}
