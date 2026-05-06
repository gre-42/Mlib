#include "Static_Position.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/OpenGL/CHK.hpp>
#include <Mlib/Scene_Graph/Instances/Billboard_Container.hpp>

using namespace Mlib;

StaticPosition::StaticPosition(
    const SortedLookatInstances& instances,
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

StaticPosition::~StaticPosition() = default;

void StaticPosition::update(const SortedLookatInstances& instances) {
    wait_and_assign(instances);
    buffer_.substitute(positions_);
}

void StaticPosition::wait_and_assign(const SortedLookatInstances& instances) {
    if (instances.size() > capacity_) {
        throw std::runtime_error("StaticPosition::wait_and_assign capacity exceeded");
    }
    buffer_.wait();
    positions_.clear();
    for (const PositionAndBillboardId<float>& m : instances) {
        positions_.push_back(m.position);
    }
}

bool StaticPosition::copy_in_progress() const {
    return buffer_.copy_in_progress();
}

void StaticPosition::wait() const {
    buffer_.wait();
}

void StaticPosition::bind(GLuint attribute_index) const {
    CHK(glEnableVertexAttribArray(attribute_index));
    buffer_.bind();
    CHK(glVertexAttribPointer(attribute_index, 3, GL_FLOAT, GL_FALSE, sizeof(Position), nullptr));
    CHK(glVertexAttribDivisor(attribute_index, 1));
}

BackgroundCopyState StaticPosition::state() const {
    return buffer_.state();
}

size_t StaticPosition::size() const {
    return positions_.size();
}
