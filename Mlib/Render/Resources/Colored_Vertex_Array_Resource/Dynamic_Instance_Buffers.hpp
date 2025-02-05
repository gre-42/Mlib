#pragma once
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Billboard_Sequence.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Dynamic_Billboard_Ids.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Dynamic_Instance_Continuous_Texture_Layer.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Dynamic_Position.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Dynamic_Position_YAngles.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Dynamic_Rotation_Quaternion.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/IInstance_Buffers.hpp>
#include <string>
#include <vector>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
struct TransformationAndBillboardId;
enum class TransformationMode;
enum class ClearOnUpdate;
struct StaticWorld;

struct InternalParticleProperties {
    FixedArray<float, 3> velocity = uninitialized;
    float air_resistance;
};

class DynamicInstanceBuffers: public IInstanceBuffers {
    DynamicInstanceBuffers(const DynamicInstanceBuffers&) = delete;
    DynamicInstanceBuffers& operator = (const DynamicInstanceBuffers&) = delete;
public:
    DynamicInstanceBuffers(
        TransformationMode transformation_mode,
        size_t max_num_instances,
        BillboardId num_billboard_atlas_components,
        bool has_per_instance_continuous_texture_layer,
        ClearOnUpdate clear_on_update);
    virtual ~DynamicInstanceBuffers() override;

    size_t num_billboard_atlas_components() const;
    void append(
        const TransformationMatrix<float, float, 3>& transformation_matrix,
        const BillboardSequence& sequence,
        const FixedArray<float, 3>& velocity,
        float air_resistance);
    void move(float dt, const StaticWorld& world);
    size_t capacity() const;
    size_t tmp_length() const;
    bool tmp_empty() const;

    // IInstanceBuffers
    virtual bool copy_in_progress() const override;
    virtual void wait() const override;
    void update();
    virtual void bind(
        GLuint instance_attribute_index,
        GLuint rotation_quaternion_attribute_index,
        GLuint billboard_ids_attribute_index,
        GLuint texture_layer_attribute_index,
        TaskLocation task_location) const override;
    virtual GLsizei num_instances() const override;
    virtual bool has_continuous_texture_layer() const override;
private:
    DynamicPositionYAngles position_yangles_;
    DynamicPosition position_;
    DynamicRotationQuaternion rotation_quaternion_;
    std::vector<InternalParticleProperties> particle_properties_;
    DynamicBillboardIds billboard_ids_;
    std::optional<DynamicInstanceContinuousTextureLayer> texture_layers_;
    size_t max_num_instances_;
    BillboardId num_billboard_atlas_components_;
    bool has_per_instance_continuous_texture_layer_;
    size_t tmp_num_instances_;
    GLsizei gl_num_instances_;
    TransformationMode transformation_mode_;
    std::vector<float> animation_times_;
    std::vector<const BillboardSequence*> billboard_sequences_;
    ClearOnUpdate clear_on_update_;
};

}
