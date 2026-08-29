#include "Static_Position_YAngles.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/OpenGL/CHK.hpp>
#include <Mlib/Scene_Graph/Instances/Billboard_Container.hpp>

using namespace Mlib;

StaticPositionYAngles::StaticPositionYAngles(
    const SortedYAngleInstances& instances,
    size_t capacity)
    // : buffer_{(capacity != 0) ? TaskLocation::BACKGROUND : TaskLocation::FOREGROUND}
{
    if (capacity != 0) {
        capacity_ = capacity;
        positions_.reserve(capacity);
        buffer_.reserve<Position>(capacity);
        wait_and_assign(instances);
        buffer_.substitute(positions_);
    } else {
        capacity_ = instances.size();
        positions_.reserve(instances.size());
        wait_and_assign(instances);
        buffer_.init(positions_);
    }
}

StaticPositionYAngles::~StaticPositionYAngles() = default;

void StaticPositionYAngles::update(const SortedYAngleInstances& instances) {
    wait_and_assign(instances);
    buffer_.substitute(positions_);
}

void StaticPositionYAngles::wait_and_assign(const SortedYAngleInstances& instances) {
    if (instances.size() > capacity_) {
        throw std::runtime_error("StaticPositionYAngles::wait_and_assign capacity exceeded");
    }
    buffer_.wait();
    positions_.clear();
    for (const PositionAndYAngleAndBillboardId<float>& m : instances) {
        positions_.emplace_back(
            m.position(0),
            m.position(1),
            m.position(2),
            m.yangle);
    }
}

bool StaticPositionYAngles::copy_in_progress() const {
    return buffer_.copy_in_progress();
}

void StaticPositionYAngles::wait() const {
    buffer_.wait();
}

void StaticPositionYAngles::bind(GLuint attribute_index) const {
    CHK(glEnableVertexAttribArray(attribute_index));
    buffer_.bind();
    CHK(glVertexAttribPointer(attribute_index, 4, GL_FLOAT, GL_FALSE, sizeof(Position), nullptr));
    CHK(glVertexAttribDivisor(attribute_index, 1));
}

BackgroundCopyState StaticPositionYAngles::state() const {
    return buffer_.state();
}

size_t StaticPositionYAngles::size() const {
    return positions_.size();
}
