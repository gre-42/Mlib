#include "Dynamic_Instance_Buffers.hpp"
#include <Mlib/Geometry/Material/Transformation_Mode.hpp>
#include <Mlib/Geometry/Mesh/Transformation_And_Billboard_Id.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Frame_Index_From_Animation_Time.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Clear_On_Update.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

DynamicInstanceBuffers::DynamicInstanceBuffers(
    TransformationMode transformation_mode,
    size_t max_num_instances,
    uint32_t num_billboard_atlas_components,
    bool has_per_instance_continuous_texture_layer,
    ClearOnUpdate clear_on_update)
    : position_yangles_{ max_num_instances }
    , position_{ max_num_instances }
    , rotation_axes_{ DynamicRotationAxis{max_num_instances, 0}, DynamicRotationAxis{max_num_instances, 1} }
    , billboard_ids_{ max_num_instances, num_billboard_atlas_components }
    , max_num_instances_{ max_num_instances }
    , num_billboard_atlas_components_{ num_billboard_atlas_components }
    , has_per_instance_continuous_texture_layer_{ has_per_instance_continuous_texture_layer }
    , tmp_num_instances_{ 0 }
    , gl_num_instances_{ 0 }
    , transformation_mode_{ transformation_mode }
    , clear_on_update_{ clear_on_update }
{
    if (num_billboard_atlas_components > 0) {
        animation_times_.resize(max_num_instances);
        billboard_sequences_.resize(max_num_instances);
    }
    if (max_num_instances > std::numeric_limits<GLsizei>::max()) {
        THROW_OR_ABORT("Maximum number of instances too large");
    }
    if (has_per_instance_continuous_texture_layer) {
        texture_layers_.emplace(max_num_instances);
    }
}

DynamicInstanceBuffers::~DynamicInstanceBuffers() = default;

size_t DynamicInstanceBuffers::num_billboard_atlas_components() const {
    return num_billboard_atlas_components_;
}

void DynamicInstanceBuffers::append(
    const TransformationMatrix<float, float, 3>& transformation_matrix,
    const BillboardSequence& sequence)
{
    if (sequence.billboard_ids.empty()) {
        THROW_OR_ABORT("Billboard sequence is empty");
    }
    if (tmp_num_instances_ == max_num_instances_) {
        THROW_OR_ABORT("Too many particles");
    }
    TransformationAndBillboardId m{
        .transformation_matrix = transformation_matrix,
        .billboard_id = sequence.billboard_ids[0]};
    std::scoped_lock lock{ mutex_ };
    if (transformation_mode_ == TransformationMode::POSITION_YANGLE) {
        position_yangles_.append(m);
    } else if ((transformation_mode_ == TransformationMode::POSITION) ||
               (transformation_mode_ == TransformationMode::POSITION_LOOKAT) ||
               (transformation_mode_ == TransformationMode::ALL))
    {
        position_.append(m);
    } else {
        THROW_OR_ABORT("Unknown transformation mode: " +  std::to_string((int)transformation_mode_));
    }
    if (transformation_mode_ == TransformationMode::ALL) {
        rotation_axes_[0].append(m);
        rotation_axes_[1].append(m);
    }
    if (num_billboard_atlas_components_ != 0) {
        billboard_ids_.append(m);
        if (has_per_instance_continuous_texture_layer_) {
            texture_layers_.value().append(0.f);
        }
        animation_times_[tmp_num_instances_] = 0.f;
        billboard_sequences_[tmp_num_instances_] = &sequence;
    }
    ++tmp_num_instances_;
}

void DynamicInstanceBuffers::move(float dt) {
    std::scoped_lock lock{ mutex_ };
    if (num_billboard_atlas_components_ == 0) {
        return;
    }
    for (size_t i = 0; i < tmp_length();) {
        auto& ai = animation_times_[i];
        auto& bi = billboard_sequences_[i];
        if (bi->duration == INFINITY) {
            ++i;
            continue;
        }
        ai += dt;
        if (ai < bi->duration) {
            auto frame_index = (size_t)frame_index_from_animation_state(
                ai,
                bi->duration,
                bi->billboard_ids.size());
            if (frame_index >= bi->billboard_ids.size()) {
                THROW_OR_ABORT("Frame index too large");
            }
            billboard_ids_[i] = bi->billboard_ids[frame_index];
            if (has_per_instance_continuous_texture_layer_) {
                texture_layers_.value()[i] = ai / bi->duration * bi->final_texture_w;
            }
            ++i;
        } else {
            if (transformation_mode_ == TransformationMode::POSITION_YANGLE) {
                position_yangles_.remove(i);
            } else if ((transformation_mode_ == TransformationMode::POSITION) ||
                       (transformation_mode_ == TransformationMode::POSITION_LOOKAT) ||
                       (transformation_mode_ == TransformationMode::ALL))
            {
                position_.remove(i);
            } else {
                THROW_OR_ABORT("Unknown transformation mode: " +  std::to_string((int)transformation_mode_));
            }
            if (transformation_mode_ == TransformationMode::ALL) {
                rotation_axes_[0].remove(i);
                rotation_axes_[1].remove(i);
            }
            billboard_ids_.remove(i);
            if (has_per_instance_continuous_texture_layer_) {
                texture_layers_.value().remove(i);
            }
            --tmp_num_instances_;
            if (tmp_num_instances_ != 0) {
                ai = animation_times_[tmp_num_instances_];
                bi = billboard_sequences_[tmp_num_instances_];
            }
        }
    }
}

size_t DynamicInstanceBuffers::capacity() const {
    return max_num_instances_;
}

size_t DynamicInstanceBuffers::tmp_length() const {
    return tmp_num_instances_;
}

bool DynamicInstanceBuffers::tmp_empty() const {
    return tmp_num_instances_ == 0;
}

bool DynamicInstanceBuffers::copy_in_progress() const {
    return false;
}

void DynamicInstanceBuffers::wait() const {
}

void DynamicInstanceBuffers::update()
{
    std::scoped_lock lock{ mutex_ };
    gl_num_instances_ = integral_cast<GLsizei>(tmp_num_instances_);
    if (tmp_num_instances_ == 0) {
        return;
    }
    if (transformation_mode_ == TransformationMode::POSITION_YANGLE) {
        position_yangles_.update();
        if (clear_on_update_ == ClearOnUpdate::YES) {
            position_yangles_.clear();
        }
    } else if ((transformation_mode_ == TransformationMode::POSITION) ||
               (transformation_mode_ == TransformationMode::POSITION_LOOKAT) ||
               (transformation_mode_ == TransformationMode::ALL))
    {
        position_.update();
        if (clear_on_update_ == ClearOnUpdate::YES) {
            position_.clear();
        }
    } else {
        THROW_OR_ABORT("Unsupported transformation mode for instances");
    }
    if (transformation_mode_ == TransformationMode::ALL) {
        rotation_axes_[0].update();
        rotation_axes_[1].update();
        if (clear_on_update_ == ClearOnUpdate::YES) {
            rotation_axes_[0].clear();
            rotation_axes_[1].clear();
        }
    }
    if (num_billboard_atlas_components_ != 0) {
        billboard_ids_.update();
        if (has_per_instance_continuous_texture_layer_) {
            texture_layers_.value().update();
        }
        if (clear_on_update_ == ClearOnUpdate::YES) {
            billboard_ids_.clear();
            if (has_per_instance_continuous_texture_layer_) {
                texture_layers_.value().clear();
            }
        }
    }
    if (clear_on_update_ == ClearOnUpdate::YES) {
        tmp_num_instances_ = 0;
    }
}

void DynamicInstanceBuffers::bind(
    GLuint instance_attribute_index,
    FixedArray<GLuint, 2> rotation_axes_attribute_index,
    GLuint billboard_ids_attribute_index,
    GLuint texture_layer_attribute_index) const
{
    std::shared_lock lock{ mutex_ };
    if (transformation_mode_ == TransformationMode::POSITION_YANGLE) {
        position_yangles_.bind(instance_attribute_index);
    } else if ((transformation_mode_ == TransformationMode::POSITION) ||
               (transformation_mode_ == TransformationMode::POSITION_LOOKAT) ||
               (transformation_mode_ == TransformationMode::ALL))
    {
        position_.bind(instance_attribute_index);
    } else {
        THROW_OR_ABORT("Unsupported transformation mode for instances");
    }
    if (transformation_mode_ == TransformationMode::ALL) {
        rotation_axes_[0].bind(rotation_axes_attribute_index(0));
        rotation_axes_[1].bind(rotation_axes_attribute_index(1));
    }
    if (num_billboard_atlas_components_ != 0) {
        billboard_ids_.bind(billboard_ids_attribute_index);
    }
    if (has_per_instance_continuous_texture_layer_) {
        texture_layers_.value().bind(texture_layer_attribute_index);
    }
}

size_t DynamicInstanceBuffers::tmp_num_instances() const {
    std::shared_lock lock{ mutex_ };
    return tmp_num_instances_;
}

GLsizei DynamicInstanceBuffers::num_instances() const {
    return gl_num_instances_;
}

bool DynamicInstanceBuffers::has_continuous_texture_layer() const {
    return has_per_instance_continuous_texture_layer_;
}
