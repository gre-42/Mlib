#include "Static_Rotation_Quaternion.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Quaternion.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/OpenGL/CHK.hpp>

using namespace Mlib;

StaticRotationQuaternion::StaticRotationQuaternion(
    const SortedTransformedInstances& instances,
    size_t capacity)
    // : buffer_{(capacity != 0) ? TaskLocation::BACKGROUND : TaskLocation::FOREGROUND}
{
    if (capacity != 0) {
        capacity_ = capacity;
        quaternions_.reserve(capacity);
        buffer_.reserve<RotationQuaternion>(capacity);
        wait_and_assign(instances);
        buffer_.substitute(quaternions_);
    } else {
        capacity_ = instances.size();
        quaternions_.reserve(instances.size());
        wait_and_assign(instances);
        buffer_.init(quaternions_);
    }
}

StaticRotationQuaternion::~StaticRotationQuaternion() = default;

void StaticRotationQuaternion::update(const SortedTransformedInstances& instances) {
    wait_and_assign(instances);
    buffer_.substitute(quaternions_);
}

void StaticRotationQuaternion::wait_and_assign(const SortedTransformedInstances& instances) {
    if (instances.size() > capacity_) {
        throw std::runtime_error("StaticRotationQuaternion::assign capacity exceeded");
    }
    buffer_.wait();
    quaternions_.clear();
    for (const TransformationMatrix<float, float, 3>& m : instances) {
        quaternions_.push_back(RotationQuaternion{ m.R });
    }
}

bool StaticRotationQuaternion::copy_in_progress() const {
    return buffer_.copy_in_progress();
}

void StaticRotationQuaternion::wait() const {
    buffer_.wait();
}

void StaticRotationQuaternion::bind(GLuint attribute_index) const {
    CHK(glEnableVertexAttribArray(attribute_index));
    buffer_.bind();
    CHK(glVertexAttribPointer(attribute_index, 4, GL_FLOAT, GL_FALSE, sizeof(RotationQuaternion), nullptr));
    CHK(glVertexAttribDivisor(attribute_index, 1));
}

BackgroundCopyState StaticRotationQuaternion::state() const {
    return buffer_.state();
}

size_t StaticRotationQuaternion::size() const {
    return quaternions_.size();
}
