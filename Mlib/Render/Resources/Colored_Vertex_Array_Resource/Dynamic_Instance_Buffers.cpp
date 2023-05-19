#include "Dynamic_Instance_Buffers.hpp"
#include <Mlib/Geometry/Mesh/Transformation_And_Billboard_Id.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Frame_Index_From_Animation_Time.hpp>
#include <Mlib/Scene_Graph/Transformation_Mode.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

DynamicInstanceBuffers::DynamicInstanceBuffers(
    TransformationMode transformation_mode,
    size_t max_num_instances,
    uint32_t num_billboard_atlas_components)
: position_yangles_{max_num_instances},
  position_{max_num_instances},
  billboard_ids_{max_num_instances, num_billboard_atlas_components},
  num_billboard_atlas_components_{num_billboard_atlas_components},
  tmp_num_instances_{0},
  gl_num_instances_{0},
  transformation_mode_{transformation_mode},
  animation_times_(max_num_instances),
  billboard_sequences_(max_num_instances)
{
    if (max_num_instances > std::numeric_limits<GLsizei>::max()) {
        THROW_OR_ABORT("Maximum number of instances too large");
    }
}

DynamicInstanceBuffers::~DynamicInstanceBuffers() = default;

void DynamicInstanceBuffers::append(
    const TransformationMatrix<float, float, 3>& transformation_matrix,
    const BillboardSequence& sequence)
{
    if (sequence.billboard_ids.empty()) {
        THROW_OR_ABORT("Billboard sequence is empty");
    }
    if (tmp_num_instances_ == animation_times_.size()) {
        THROW_OR_ABORT("Too many particles");
    }
    TransformationAndBillboardId m{
        .transformation_matrix = transformation_matrix,
        .billboard_id = sequence.billboard_ids[0]};
    std::scoped_lock lock{mutex_};
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
    animation_times_[tmp_num_instances_] = 0.f;
    billboard_sequences_[tmp_num_instances_] = &sequence;
    ++tmp_num_instances_;
}

void DynamicInstanceBuffers::move(float dt) {
    std::scoped_lock lock{mutex_};
    for (size_t i = 0; i < tmp_length();) {
        auto& ai = animation_times_[i];
        auto& bi = billboard_sequences_[i];
        ai += dt;
        if (ai <= bi->duration) {
            auto frame_index = (size_t)frame_index_from_animation_state(
                ai,
                bi->duration,
                bi->billboard_ids.size());
            if (frame_index >= bi->billboard_ids.size()) {
                THROW_OR_ABORT("Frame index too large");
            }
            billboard_ids_.modify(i, bi->billboard_ids[frame_index]);
            ++i;
        } else {
            --tmp_num_instances_;
            if (tmp_num_instances_ == 0) {
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
            ai = animation_times_[tmp_num_instances_];
            bi = billboard_sequences_[tmp_num_instances_];
        }
    }
}

size_t DynamicInstanceBuffers::capacity() const {
    return animation_times_.size();
}

size_t DynamicInstanceBuffers::tmp_length() const {
    return tmp_num_instances_;
}

bool DynamicInstanceBuffers::tmp_empty() const {
    return tmp_num_instances_ == 0;
}

void DynamicInstanceBuffers::update()
{
    std::scoped_lock lock{mutex_};
    if (transformation_mode_ == TransformationMode::POSITION_YANGLE) {
        position_yangles_.update();
    } else if ((transformation_mode_ == TransformationMode::POSITION) ||
               (transformation_mode_ == TransformationMode::POSITION_LOOKAT))
    {
        position_.update();
    } else {
        THROW_OR_ABORT("Unsupported transformation mode for instances");
    }
    if (num_billboard_atlas_components_ != 0) {
        billboard_ids_.update();
    }
    gl_num_instances_ = integral_cast<GLsizei>(tmp_num_instances_);
}

void DynamicInstanceBuffers::bind(
    GLuint instance_attribute_index,
    GLuint billboard_ids_attribute_index) const
{
    std::shared_lock lock{mutex_};
    if (transformation_mode_ == TransformationMode::POSITION_YANGLE) {
        position_yangles_.bind(instance_attribute_index);
    } else if ((transformation_mode_ == TransformationMode::POSITION) ||
                (transformation_mode_ == TransformationMode::POSITION_LOOKAT))
    {
        position_.bind(instance_attribute_index);
    } else {
        THROW_OR_ABORT("Unsupported transformation mode for instances");
    }
    if (num_billboard_atlas_components_ != 0) {
        billboard_ids_.bind(billboard_ids_attribute_index);
    }
}

GLsizei DynamicInstanceBuffers::num_instances() const {
    return gl_num_instances_;
}
