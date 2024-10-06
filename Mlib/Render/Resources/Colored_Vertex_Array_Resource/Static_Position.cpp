#include "Static_Position.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Mesh/Transformation_And_Billboard_Id.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Render/CHK.hpp>

using namespace Mlib;

StaticPosition::StaticPosition(const std::vector<TransformationAndBillboardId>& instances)
    : instances_{ instances }
{
    positions_.reserve(instances_.size());
    for (const TransformationAndBillboardId& m : instances_) {
        positions_.push_back(m.transformation_matrix.t);
    }
}

StaticPosition::~StaticPosition() = default;

bool StaticPosition::copy_in_progress() const {
    return buffer_.copy_in_progress();
}

void StaticPosition::wait() const {
    buffer_.wait();
}

void StaticPosition::bind(GLuint attribute_index, TaskLocation task_location) const {
    buffer_.set(positions_, task_location);

    CHK(glEnableVertexAttribArray(attribute_index));
    CHK(glVertexAttribPointer(attribute_index, 3, GL_FLOAT, GL_FALSE, sizeof(Position), nullptr));
    CHK(glVertexAttribDivisor(attribute_index, 1));
}
