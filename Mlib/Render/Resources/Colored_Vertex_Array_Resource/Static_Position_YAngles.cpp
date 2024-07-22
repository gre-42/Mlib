#include "Static_Position_YAngles.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Mesh/Transformation_And_Billboard_Id.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Render/CHK.hpp>

using namespace Mlib;

StaticPositionYAngles::StaticPositionYAngles(const std::vector<TransformationAndBillboardId>& instances)
: instances_{instances}
{
    positions_.reserve(instances_.size());
    for (const TransformationAndBillboardId &m : instances_) {
        positions_.emplace_back(
            m.transformation_matrix.t(0),
            m.transformation_matrix.t(1),
            m.transformation_matrix.t(2),
            std::atan2(-m.transformation_matrix.R(2, 0), m.transformation_matrix.R(0, 0)));
    }
}

StaticPositionYAngles::~StaticPositionYAngles() = default;

bool StaticPositionYAngles::copy_in_progress() const {
    return buffer_.copy_in_progress();
}

void StaticPositionYAngles::wait() const {
    buffer_.wait();
}

void StaticPositionYAngles::bind(GLuint attribute_index, TaskLocation task_location) const {
    buffer_.set(positions_, task_location);

    CHK(glEnableVertexAttribArray(attribute_index));
    CHK(glVertexAttribPointer(attribute_index, 4, GL_FLOAT, GL_FALSE, sizeof(Position), nullptr));
    CHK(glVertexAttribDivisor(attribute_index, 1));
}
