#pragma once
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/IInstance_Buffers.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Static_Billboard_Ids.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Static_Position.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Static_Position_YAngles.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Static_Rotation_Quaternion.hpp>
#include <string>
#include <vector>

namespace Mlib {

struct TransformationAndBillboardId;
enum class TransformationMode;

class StaticInstanceBuffers: public IInstanceBuffers {
    StaticInstanceBuffers(const StaticInstanceBuffers&) = delete;
    StaticInstanceBuffers& operator = (const StaticInstanceBuffers&) = delete;
public:
    StaticInstanceBuffers(
        TransformationMode transformation_mode,
        std::vector<TransformationAndBillboardId>&& instances,
        BillboardId num_billboard_atlas_components,
        const std::string& name);
    virtual ~StaticInstanceBuffers() override;
    virtual bool copy_in_progress() const override;
    virtual void wait() const override;
    virtual void update() override;
    virtual void bind(
        GLuint instance_attribute_index,
        GLuint rotation_quaternion_attribute_index,
        GLuint billboard_ids_attribute_index,
        GLuint texture_layer_attribute_index,
        TaskLocation task_location) const override;
    virtual size_t tmp_num_instances() const override;
    virtual GLsizei num_instances() const override;
    virtual bool has_continuous_texture_layer() const override;
private:
    std::vector<TransformationAndBillboardId> instances_;
    StaticPositionYAngles position_yangles_;
    StaticPosition position_;
    StaticRotationQuaternion rotation_quaternion;
    StaticBillboardIds billboard_ids_;
    BillboardId num_billboard_atlas_components_;
    TransformationMode transformation_mode_;
};

}
