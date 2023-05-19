#pragma once
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/IInstance_Buffers.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Static_Billboard_Ids.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Static_Position.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Static_Position_YAngles.hpp>
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
        uint32_t num_billboard_atlas_components,
        const std::string& name);
    virtual ~StaticInstanceBuffers() override;
    virtual void update() override;
    virtual void bind(
        GLuint instance_attribute_index,
        GLuint billboard_ids_attribute_index) const override;
    virtual GLsizei num_instances() const override;
private:
    std::vector<TransformationAndBillboardId> instances_;
    StaticPositionYAngles position_yangles_;
    StaticPosition position_;
    StaticBillboardIds billboard_ids_;
    uint32_t num_billboard_atlas_components_;
    TransformationMode transformation_mode_;
};

}
