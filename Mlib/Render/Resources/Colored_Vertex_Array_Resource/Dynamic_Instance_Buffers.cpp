#include "Dynamic_Instance_Buffers.hpp"
#include <Mlib/Geometry/Material/Transformation_Mode.hpp>
#include <Mlib/Geometry/Mesh/Transformation_And_Billboard_Id.hpp>
#include <Mlib/Math/Lerp.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Frame_Index_From_Animation_Time.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Clear_On_Update.hpp>
#include <Mlib/Scene_Graph/Batch_Renderers/Task_Location.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>
#include <unordered_map>

using namespace Mlib;

DynamicInstanceBuffers::DynamicInstanceBuffers(
    TransformationMode transformation_mode,
    size_t max_num_instances,
    BillboardId num_billboard_atlas_components,
    bool has_per_instance_continuous_texture_layer,
    ClearOnUpdate clear_on_update)
    : position_yangles_{ max_num_instances }
    , position_{ max_num_instances }
    , rotation_quaternion_{ max_num_instances }
    , particle_properties_{ max_num_instances }
    , billboard_ids_{ max_num_instances, num_billboard_atlas_components }
    , max_num_instances_{ max_num_instances }
    , num_billboard_atlas_components_{ num_billboard_atlas_components }
    , has_per_instance_continuous_texture_layer_{ has_per_instance_continuous_texture_layer }
    , tmp_num_instances_{ 0 }
    , gl_num_instances_{ 0 }
    , transformation_mode_{ transformation_mode }
    , clear_on_update_{ clear_on_update }
    , wind_vector_{ fixed_zeros<float, 3>() }
    , latest_update_time_{ std::chrono::steady_clock::time_point() }
    , latest_update_time_id_{ RENDER_TIME_ID_END }
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
    const BillboardSequence& sequence,
    const FixedArray<float, 3>& velocity,
    float air_resistance_halflife,
    float texture_layer)
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
    if (transformation_mode_ == TransformationMode::POSITION_YANGLE) {
        position_yangles_.append(m);
    } else if ((transformation_mode_ == TransformationMode::POSITION) ||
               (transformation_mode_ == TransformationMode::POSITION_FLAT) ||
               (transformation_mode_ == TransformationMode::POSITION_LOOKAT) ||
               (transformation_mode_ == TransformationMode::ALL))
    {
        position_.append(m);
    } else {
        THROW_OR_ABORT("Unknown transformation mode: " +  std::to_string((int)transformation_mode_));
    }
    if (transformation_mode_ == TransformationMode::ALL) {
        rotation_quaternion_.append(m);
    }
    particle_properties_[tmp_num_instances_] = { velocity, air_resistance_halflife };
    if (num_billboard_atlas_components_ != 0) {
        billboard_ids_.append(m);
        if (has_per_instance_continuous_texture_layer_) {
            texture_layers_->append(texture_layer);
        }
        animation_times_[tmp_num_instances_] = 0.f;
        billboard_sequences_[tmp_num_instances_] = &sequence;
    }
    ++tmp_num_instances_;
}

void DynamicInstanceBuffers::set_wind_vector(const FixedArray<float, 3>& wind_vector) {
    wind_vector_ = wind_vector_;
}

void DynamicInstanceBuffers::move_renderables(float dt) {
    if (num_billboard_atlas_components_ == 0) {
        return;
    }
    if (clear_on_update_ == ClearOnUpdate::YES) {
        return;
    }
    std::unordered_map<float, float> air_resistances;
    for (size_t i = 0; i < tmp_length();) {
        auto& ai = animation_times_[i];
        auto& bi = billboard_sequences_[i];
        auto& props = particle_properties_[i];
        auto advance_position = [&](FixedArray<float, 3>& position) {
            position += props.velocity * dt;
            if (props.air_resistance_halflife != INFINITY) {
                auto it = air_resistances.find(props.air_resistance_halflife);
                if (it == air_resistances.end()) {
                    auto res = air_resistances.try_emplace(props.air_resistance_halflife, std::pow(0.5f, dt / props.air_resistance_halflife));
                    if (!res.second) {
                        verbose_abort("DynamicInstanceBuffers::move_renderables internal error");
                    }
                    it = res.first;
                }
                props.velocity = lerp(props.velocity, wind_vector_, 1.f - it->second);
            }
        };
        if (transformation_mode_ == TransformationMode::POSITION_YANGLE) {
            advance_position(position_yangles_[i].row_range<0, 3>());
        } else if ((transformation_mode_ == TransformationMode::POSITION) ||
                   (transformation_mode_ == TransformationMode::POSITION_FLAT) ||
                   (transformation_mode_ == TransformationMode::POSITION_LOOKAT) ||
                   (transformation_mode_ == TransformationMode::ALL))
        {
            advance_position(position_[i]);
        } else {
            THROW_OR_ABORT("Unknown transformation mode: " +  std::to_string((int)transformation_mode_));
        }
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
                (*texture_layers_)[i] = ai / bi->duration * bi->final_texture_w;
            }
            ++i;
        } else {
            if (transformation_mode_ == TransformationMode::POSITION_YANGLE) {
                position_yangles_.remove(i);
            } else if ((transformation_mode_ == TransformationMode::POSITION) ||
                       (transformation_mode_ == TransformationMode::POSITION_FLAT) ||
                       (transformation_mode_ == TransformationMode::POSITION_LOOKAT) ||
                       (transformation_mode_ == TransformationMode::ALL))
            {
                position_.remove(i);
            } else {
                THROW_OR_ABORT("Unknown transformation mode: " +  std::to_string((int)transformation_mode_));
            }
            if (transformation_mode_ == TransformationMode::ALL) {
                rotation_quaternion_.remove(i);
            }
            billboard_ids_.remove(i);
            if (has_per_instance_continuous_texture_layer_) {
                texture_layers_->remove(i);
            }
            --tmp_num_instances_;
            if (tmp_num_instances_ != 0) {
                ai = animation_times_[tmp_num_instances_];
                bi = billboard_sequences_[tmp_num_instances_];
                props = particle_properties_[tmp_num_instances_];
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

void DynamicInstanceBuffers::update(
    std::chrono::steady_clock::time_point time,
    RenderTimeId time_id)
{
    if ((clear_on_update_ == ClearOnUpdate::YES) && (time_id == latest_update_time_id_)) {
        return;
    }
    latest_update_time_id_ = time_id;
    gl_num_instances_ = integral_cast<GLsizei>(tmp_num_instances_);
    if (tmp_num_instances_ == 0) {
        return;
    }
    if (latest_update_time_ == std::chrono::steady_clock::time_point()) {
        latest_update_time_ = time;
        return;
    } else if (latest_update_time_ == time) {
        return;
    } else {
        move_renderables(std::chrono::duration<float>(time - latest_update_time_).count() * seconds);
        latest_update_time_ = time;
    }
    if (transformation_mode_ == TransformationMode::POSITION_YANGLE) {
        position_yangles_.update();
        if (clear_on_update_ == ClearOnUpdate::YES) {
            position_yangles_.clear();
        }
    } else if ((transformation_mode_ == TransformationMode::POSITION) ||
               (transformation_mode_ == TransformationMode::POSITION_FLAT) ||
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
        rotation_quaternion_.update();
        if (clear_on_update_ == ClearOnUpdate::YES) {
            rotation_quaternion_.clear();
        }
    }
    if (num_billboard_atlas_components_ != 0) {
        billboard_ids_.update();
        if (has_per_instance_continuous_texture_layer_) {
            texture_layers_->update();
        }
        if (clear_on_update_ == ClearOnUpdate::YES) {
            billboard_ids_.clear();
            if (has_per_instance_continuous_texture_layer_) {
                texture_layers_->clear();
            }
        }
    }
    if (clear_on_update_ == ClearOnUpdate::YES) {
        tmp_num_instances_ = 0;
    }
}

void DynamicInstanceBuffers::bind(
    GLuint instance_attribute_index,
    GLuint rotation_quaternion_attribute_index,
    GLuint billboard_ids_attribute_index,
    GLuint texture_layer_attribute_index,
    TaskLocation task_location) const
{
    if (task_location != TaskLocation::FOREGROUND) {
        THROW_OR_ABORT("DynamicInstanceBuffers only supports foreground tasks");
    }
    if (transformation_mode_ == TransformationMode::POSITION_YANGLE) {
        position_yangles_.bind(instance_attribute_index);
    } else if ((transformation_mode_ == TransformationMode::POSITION) ||
               (transformation_mode_ == TransformationMode::POSITION_FLAT) ||
               (transformation_mode_ == TransformationMode::POSITION_LOOKAT) ||
               (transformation_mode_ == TransformationMode::ALL))
    {
        position_.bind(instance_attribute_index);
    } else {
        THROW_OR_ABORT("Unsupported transformation mode for instances");
    }
    if (transformation_mode_ == TransformationMode::ALL) {
        rotation_quaternion_.bind(rotation_quaternion_attribute_index);
    }
    if (num_billboard_atlas_components_ != 0) {
        billboard_ids_.bind(billboard_ids_attribute_index);
    }
    if (has_per_instance_continuous_texture_layer_) {
        texture_layers_->bind(texture_layer_attribute_index);
    }
}

GLsizei DynamicInstanceBuffers::num_instances() const {
    return gl_num_instances_;
}

bool DynamicInstanceBuffers::has_continuous_texture_layer() const {
    return has_per_instance_continuous_texture_layer_;
}
