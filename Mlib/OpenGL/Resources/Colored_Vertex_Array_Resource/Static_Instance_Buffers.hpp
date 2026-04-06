#pragma once
#include <Mlib/OpenGL/Resources/Colored_Vertex_Array_Resource/Static_Billboard_Ids.hpp>
#include <Mlib/OpenGL/Resources/Colored_Vertex_Array_Resource/Static_Position.hpp>
#include <Mlib/OpenGL/Resources/Colored_Vertex_Array_Resource/Static_Position_YAngles.hpp>
#include <Mlib/OpenGL/Resources/Colored_Vertex_Array_Resource/Static_Rotation_Quaternion.hpp>
#include <Mlib/Scene_Graph/Instances/Sorted_Vertex_Array_Instances.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Instance_Buffers.hpp>
#include <optional>
#include <string>
#include <vector>

namespace Mlib {

class StaticInstanceBuffers: public IGpuInstanceBuffers {
    StaticInstanceBuffers(const StaticInstanceBuffers&) = delete;
    StaticInstanceBuffers& operator = (const StaticInstanceBuffers&) = delete;
public:
    StaticInstanceBuffers(
        TransformationMode transformation_mode,
        const SortedVertexArrayInstances& instances,
        size_t capacity,
        BillboardId num_billboard_atlas_components,
        const std::string& name);
    virtual ~StaticInstanceBuffers() override;
    virtual void update(const SortedVertexArrayInstances& instances) override;
    virtual bool copy_in_progress() const override;
    virtual void wait() const override;
    virtual void bind(
        uint32_t idx_instance_attrs,
        uint32_t idx_rotation_quaternion,
        uint32_t idx_billboard_ids,
        uint32_t idx_texture_layer) const override;
    virtual size_t num_instances() const override;
    virtual bool has_continuous_texture_layer() const override;
    virtual void print_stats(std::ostream& ostr) const override;
private:
    std::optional<StaticPositionYAngles> position_yangles_;
    std::optional<StaticPosition> position_;
    std::optional<StaticRotationQuaternion> rotation_quaternion_;
    std::optional<StaticBillboardIds> billboard_ids_;
    BillboardId num_billboard_atlas_components_;
    TransformationMode transformation_mode_;
};

}
