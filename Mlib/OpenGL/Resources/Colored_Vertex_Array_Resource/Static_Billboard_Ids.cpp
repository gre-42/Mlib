
#include "Static_Billboard_Ids.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Material/Transformation_Mode.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/OpenGL/CHK.hpp>
#include <Mlib/Scene_Graph/Instances/Billboard_Container.hpp>

using namespace Mlib;

StaticBillboardIds::StaticBillboardIds(
    TransformationMode transformation_mode,
    const SortedVertexArrayInstances& instances,
    BillboardId num_billboard_atlas_components,
    size_t capacity)
    : transformation_mode_{ transformation_mode }
    , num_billboard_atlas_components_{ num_billboard_atlas_components }
    // , buffer_{(capacity != 0) ? TaskLocation::BACKGROUND : TaskLocation::FOREGROUND}
{
    if (capacity != 0) {
        capacity_ = capacity;
        billboard_ids_.reserve(capacity);
        buffer_.reserve<BillboardId>(capacity);
        wait_and_assign(instances);
        buffer_.substitute(billboard_ids_);
    } else {
        [&](){
            switch (transformation_mode_) {
            case TransformationMode::ALL:
                throw std::runtime_error("TransformationMode::ALL does not support billboards");
            case TransformationMode::POSITION_FLAT:
            case TransformationMode::POSITION_LOOKAT:
            case TransformationMode::POSITION:
                capacity_ = instances.lookat.size();
                billboard_ids_.reserve(instances.lookat.size());
                return;
            case TransformationMode::POSITION_YANGLE:
                capacity_ = instances.yangle.size();
                billboard_ids_.reserve(instances.yangle.size());
                return;
            }
            throw std::runtime_error("Unsupported transformation mode for instances");
        }();
        wait_and_assign(instances);
        buffer_.init(billboard_ids_);
    }
}

StaticBillboardIds::~StaticBillboardIds() = default;

void StaticBillboardIds::update(const SortedVertexArrayInstances& instances) {
    wait_and_assign(instances);
    buffer_.substitute(billboard_ids_);
}

void StaticBillboardIds::wait_and_assign(const SortedVertexArrayInstances& instances) {
    if (num_billboard_atlas_components_ == 0) {
        return;
    }
    auto add = [this](const auto& instances){
        if (instances.size() > capacity_) {
            throw std::runtime_error("StaticBillboardIds::wait_and_assign capacity exceeded");
        }
        buffer_.wait();
        billboard_ids_.clear();
        for (const auto& m : instances) {
            if (m.billboard_id >= num_billboard_atlas_components_) {
                throw std::runtime_error("Billboard ID too large");
            }
            billboard_ids_.push_back(m.billboard_id);
        }
    };
    switch (transformation_mode_) {
    case TransformationMode::ALL:
        throw std::runtime_error("TransformationMode::ALL does not support billboards");
    case TransformationMode::POSITION_FLAT:
    case TransformationMode::POSITION_LOOKAT:
    case TransformationMode::POSITION:
        add(instances.lookat);
        return;
    case TransformationMode::POSITION_YANGLE:
        add(instances.yangle);
        return;
    }
    throw std::runtime_error("Unsupported transformation mode for instances");
}

bool StaticBillboardIds::copy_in_progress() const {
    if (num_billboard_atlas_components_ == 0) {
        return false;
    }
    return buffer_.copy_in_progress();
}

void StaticBillboardIds::wait() const {
    buffer_.wait();
}

void StaticBillboardIds::bind(GLuint attribute_index) const
{
    if (num_billboard_atlas_components_ == 0) {
        throw std::runtime_error("Attempt to bind zero billboard-IDs");
    }
    CHK(glEnableVertexAttribArray(attribute_index));
    buffer_.bind();
    static_assert(std::is_same_v<BillboardId, uint16_t>);
    CHK(glVertexAttribIPointer(attribute_index, 1, GL_UNSIGNED_SHORT, sizeof(BillboardId), nullptr));
    CHK(glVertexAttribDivisor(attribute_index, 1));
}

BackgroundCopyState StaticBillboardIds::state() const {
    return buffer_.state();
}

size_t StaticBillboardIds::size() const {
    return billboard_ids_.size();
}
