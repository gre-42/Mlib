#include "Dynamic_Instance_Buffers.hpp"
#include <Mlib/Geometry/Mesh/Transformation_And_Billboard_Id.hpp>
#include <Mlib/Geometry/Mesh/Transformation_And_Billboard_Id.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Scene_Graph/Transformation_Mode.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

DynamicInstanceBuffers::DynamicInstanceBuffers(
    TransformationMode transformation_mode,
    GLsizei max_num_instances,
    uint32_t num_billboard_atlas_components,
    const std::string& name)
: position_yangles_{max_num_instances},
  position_{max_num_instances},
  billboard_ids_{max_num_instances, num_billboard_atlas_components},
  num_instances_{0},
  transformation_mode_{transformation_mode},
  animation_times_(integral_cast<size_t>(max_num_instances)),
  billboard_sequences_(integral_cast<size_t>(max_num_instances))
{}

DynamicInstanceBuffers::~DynamicInstanceBuffers() = default;

void DynamicInstanceBuffers::append(
    const TransformationMatrix<float, float, 3>& transformation_matrix,
    const BillboardSequence& sequence)
{
    if (sequence.billboard_ids.empty()) {
        THROW_OR_ABORT("Billboard sequence is empty");
    }
    if (num_instances_ == animation_times_.size()) {
        THROW_OR_ABORT("Too many particles");
    }
    TransformationAndBillboardId m{
        .transformation_matrix = transformation_matrix,
        .billboard_id = sequence.billboard_ids[0]};
    if (transformation_mode_ == TransformationMode::POSITION_YANGLE) {
        position_yangles_.append(m);
    } else if ((transformation_mode_ == TransformationMode::POSITION) ||
               (transformation_mode_ == TransformationMode::POSITION_LOOKAT))
    {
        position_.append(m);
    } else {
        THROW_OR_ABORT("Unknown transformation mode: " +  std::to_string((int)transformation_mode_));
    }
    billboard_ids_.append(m);
    animation_times_[integral_cast<size_t>(num_instances_)] = 0.f;
    billboard_sequences_[integral_cast<size_t>(num_instances_)] = &sequence;
    ++num_instances_;
}

void DynamicInstanceBuffers::advance_time(float dt) {
    for (GLsizei i = 0; i < length(); ++i) {
        size_t si = integral_cast<size_t>(i);
        animation_times_[si] += dt;
        if (animation_times_[si] > billboard_sequences_[si]->duration) {
            --num_instances_;
            if (num_instances_ == 0) {
                break;
            }
            if (transformation_mode_ == TransformationMode::POSITION_YANGLE) {
                position_yangles_.remove(i);
            } else if ((transformation_mode_ == TransformationMode::POSITION) ||
                       (transformation_mode_ == TransformationMode::POSITION_LOOKAT))
            {
                position_.remove(i);
            } else {
                THROW_OR_ABORT("Unknown transformation mode: " +  std::to_string((int)transformation_mode_));
            }
            billboard_ids_.remove(i);
            animation_times_[si] = animation_times_[integral_cast<size_t>(num_instances_)];
            billboard_sequences_[si] = billboard_sequences_[integral_cast<size_t>(num_instances_)];
        }
    }
}

GLsizei DynamicInstanceBuffers::capacity() const {
    return integral_cast<GLsizei>(animation_times_.size());
}

GLsizei DynamicInstanceBuffers::length() const {
    return num_instances_;
}

bool DynamicInstanceBuffers::empty() const {
    return num_instances_ == 0;
}

void DynamicInstanceBuffers::bind_position_yangles(GLuint attribute_index) const {
    position_yangles_.bind(attribute_index);
}

void DynamicInstanceBuffers::bind_position(GLuint attribute_index) const {
    position_.bind(attribute_index);
}

void DynamicInstanceBuffers::bind_billboard_atlas_instances(GLuint attribute_index) const
{
    billboard_ids_.bind(attribute_index);
}

GLsizei DynamicInstanceBuffers::num_instances() const {
    return num_instances_;
}
