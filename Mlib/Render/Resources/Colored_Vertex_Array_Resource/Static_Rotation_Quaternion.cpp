#include "Static_Rotation_Quaternion.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Mesh/Transformation_And_Billboard_Id.hpp>
#include <Mlib/Math/Transformation/Quaternion.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Render/CHK.hpp>

using namespace Mlib;

StaticRotationQuaternion::StaticRotationQuaternion(
    const std::vector<TransformationAndBillboardId>& instances)
    : instances_{ instances }
{
    quaternions_.reserve(instances_.size());
    for (const TransformationAndBillboardId& m : instances_) {
        quaternions_.push_back(RotationQuaternion{ m.transformation_matrix.R });
    }
}

StaticRotationQuaternion::~StaticRotationQuaternion() = default;

bool StaticRotationQuaternion::copy_in_progress() const {
    return buffer_.copy_in_progress();
}

void StaticRotationQuaternion::wait() const {
    buffer_.wait();
}

void StaticRotationQuaternion::bind(GLuint attribute_index, TaskLocation task_location) const {
    buffer_.set(quaternions_, task_location);

    CHK(glEnableVertexAttribArray(attribute_index));
    CHK(glVertexAttribPointer(attribute_index, 4, GL_FLOAT, GL_FALSE, sizeof(RotationQuaternion), nullptr));
    CHK(glVertexAttribDivisor(attribute_index, 1));
}
