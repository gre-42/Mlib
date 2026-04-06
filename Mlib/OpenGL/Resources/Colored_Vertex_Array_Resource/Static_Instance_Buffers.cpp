#include "Static_Instance_Buffers.hpp"
#include <Mlib/Geometry/Material/Transformation_Mode.hpp>
#include <stdexcept>
#include <unordered_set>

using namespace Mlib;

StaticInstanceBuffers::StaticInstanceBuffers(
    TransformationMode transformation_mode,
    const SortedVertexArrayInstances& instances,
    size_t capacity,
    BillboardId num_billboard_atlas_components,
    const std::string& name)
    : num_billboard_atlas_components_{ num_billboard_atlas_components }
    , transformation_mode_{ transformation_mode }
{
    [&](){
        switch (transformation_mode_) {
        case TransformationMode::POSITION_YANGLE:
            position_yangles_.emplace(instances.yangle, capacity);
            return;
        case TransformationMode::POSITION:
        case TransformationMode::POSITION_FLAT:
        case TransformationMode::POSITION_LOOKAT:
        case TransformationMode::ALL:
            position_.emplace(instances.lookat, capacity);
            return;
        }
        throw std::runtime_error("Unsupported transformation mode for instances");
    }();
    if (transformation_mode_ == TransformationMode::ALL) {
        rotation_quaternion_.emplace(instances.transformed, capacity);
    }
    if (num_billboard_atlas_components_ != 0) {
        billboard_ids_.emplace(transformation_mode, instances, num_billboard_atlas_components, capacity);
    }
    if (capacity == 0) {
        switch (transformation_mode_) {
        case TransformationMode::ALL:
            if (instances.transformed.empty()) {
                throw std::runtime_error("StaticInstanceBuffers::StaticInstanceBuffers received empty transformation instances \"" + name + '"');
            }
            return;
        case TransformationMode::POSITION_FLAT:
        case TransformationMode::POSITION_LOOKAT:
        case TransformationMode::POSITION:
            if (instances.lookat.empty()) {
                throw std::runtime_error("StaticInstanceBuffers::StaticInstanceBuffers received empty position instances \"" + name + '"');
            }
            return;
        case TransformationMode::POSITION_YANGLE:
            if (instances.yangle.empty()) {
                throw std::runtime_error("StaticInstanceBuffers::StaticInstanceBuffers received empty yangle instances \"" + name + '"');
            }
            return;
        }
        throw std::runtime_error("Unsupported transformation mode for instances");
    }
}

StaticInstanceBuffers::~StaticInstanceBuffers() = default;

bool StaticInstanceBuffers::copy_in_progress() const {
    if (transformation_mode_ == TransformationMode::POSITION_YANGLE) {
        if (position_yangles_.value().copy_in_progress()) {
            return true;
        }
    } else if ((transformation_mode_ == TransformationMode::POSITION) ||
               (transformation_mode_ == TransformationMode::POSITION_FLAT) ||
               (transformation_mode_ == TransformationMode::POSITION_LOOKAT) ||
               (transformation_mode_ == TransformationMode::ALL)) {
        if (position_.value().copy_in_progress()) {
            return true;
        }
    } else {
        throw std::runtime_error("Unsupported transformation mode for instances");
    }
    if (transformation_mode_ == TransformationMode::ALL) {
        if (rotation_quaternion_.value().copy_in_progress()) {
            return true;
        }
    }
    if (num_billboard_atlas_components_ != 0) {
        if (billboard_ids_.value().copy_in_progress()) {
            return true;
        }
    }
    return false;
}

void StaticInstanceBuffers::wait() const {
    if (transformation_mode_ == TransformationMode::POSITION_YANGLE) {
        position_yangles_.value().wait();
    } else if ((transformation_mode_ == TransformationMode::POSITION) ||
               (transformation_mode_ == TransformationMode::POSITION_FLAT) ||
               (transformation_mode_ == TransformationMode::POSITION_LOOKAT) ||
               (transformation_mode_ == TransformationMode::ALL))
    {
        position_.value().wait();
    } else {
        throw std::runtime_error("Unsupported transformation mode for instances");
    }
    if (transformation_mode_ == TransformationMode::ALL) {
        rotation_quaternion_.value().wait();
    }
    if (num_billboard_atlas_components_ != 0) {
        billboard_ids_.value().wait();
    }
}

void StaticInstanceBuffers::update(const SortedVertexArrayInstances& instances) {
    if (transformation_mode_ == TransformationMode::POSITION_YANGLE) {
        position_yangles_.value().update(instances.yangle);
    } else if ((transformation_mode_ == TransformationMode::POSITION) ||
               (transformation_mode_ == TransformationMode::POSITION_FLAT) ||
               (transformation_mode_ == TransformationMode::POSITION_LOOKAT) ||
               (transformation_mode_ == TransformationMode::ALL))
    {
        position_.value().update(instances.lookat);
    } else {
        throw std::runtime_error("Unsupported transformation mode for instances");
    }
    if (transformation_mode_ == TransformationMode::ALL) {
        rotation_quaternion_.value().update(instances.transformed);
    }
    if (num_billboard_atlas_components_ != 0) {
        billboard_ids_.value().update(instances);
    }
}

void StaticInstanceBuffers::bind(
    uint32_t idx_instance_attrs,
    uint32_t idx_rotation_quaternion,
    uint32_t idx_billboard_ids,
    uint32_t idx_texture_layer) const
{
    std::unordered_set<uint32_t> occupied_slots;
    auto insert_into_slot = [&occupied_slots](uint32_t index){
        if (!occupied_slots.insert(index).second) {
            throw std::runtime_error("Slot already occupied");
        }
    };
    if (transformation_mode_ == TransformationMode::POSITION_YANGLE) {
        insert_into_slot(idx_instance_attrs);
        position_yangles_.value().bind(idx_instance_attrs);
    } else if ((transformation_mode_ == TransformationMode::POSITION) ||
               (transformation_mode_ == TransformationMode::POSITION_FLAT) ||
               (transformation_mode_ == TransformationMode::POSITION_LOOKAT) ||
               (transformation_mode_ == TransformationMode::ALL))
    {
        insert_into_slot(idx_instance_attrs);
        position_.value().bind(idx_instance_attrs);
    } else {
        throw std::runtime_error("Unsupported transformation mode for instances");
    }
    if (transformation_mode_ == TransformationMode::ALL) {
        insert_into_slot(idx_rotation_quaternion);
        rotation_quaternion_.value().bind(idx_rotation_quaternion);
    }
    if (num_billboard_atlas_components_ != 0) {
        insert_into_slot(idx_billboard_ids);
        billboard_ids_.value().bind(idx_billboard_ids);
    }
}

size_t StaticInstanceBuffers::num_instances() const {
    switch (transformation_mode_) {
    case TransformationMode::ALL:
        if (rotation_quaternion_.value().size() != position_.value().size()) {
            throw std::runtime_error("Number of rotation quaternions differs from number of positions");
        }
        return rotation_quaternion_.value().size();
    case TransformationMode::POSITION_FLAT:
    case TransformationMode::POSITION_LOOKAT:
    case TransformationMode::POSITION:
        return position_.value().size();
    case TransformationMode::POSITION_YANGLE:
        return position_yangles_.value().size();
    }
    throw std::runtime_error("Unsupported transformation mode for instances");
}

bool StaticInstanceBuffers::has_continuous_texture_layer() const {
    return false;
}

void StaticInstanceBuffers::print_stats(std::ostream& ostr) const {
    ostr << "StaticInstanceBuffers" << '\n';
    if (position_yangles_.has_value()) {
        ostr << "  #position_yangles: " << position_yangles_->size() << ", " << background_copy_state_to_string(position_yangles_->state()) << '\n';
    }
    if (position_.has_value()) {
        ostr << "  #position: " << position_->size() << ", " << background_copy_state_to_string(position_->state()) << '\n';
    }
    if (rotation_quaternion_.has_value()) {
        ostr << "  #rotation_quaternion: " << rotation_quaternion_->size() << ", " << background_copy_state_to_string(rotation_quaternion_->state()) << '\n';
    }
    if (billboard_ids_.has_value()) {
        ostr << "  #billboard_ids: " << billboard_ids_->size() << ", " << background_copy_state_to_string(billboard_ids_->state()) << '\n';
    }
    ostr << "  #num_billboard_atlas_components: " << num_billboard_atlas_components_ << '\n';
    ostr << "  #transformation_mode: " << transformation_mode_to_string(transformation_mode_) << '\n';
}
