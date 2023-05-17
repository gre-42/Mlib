#pragma once
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/IInstance_Buffers.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Static_Billboard_Ids.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Static_Position.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Static_Position_YAngles.hpp>
#include <string>
#include <vector>

namespace Mlib {

struct TransformationAndBillboardId;

class StaticInstanceBuffers: public IInstanceBuffers {
    StaticInstanceBuffers(const StaticInstanceBuffers&) = delete;
    StaticInstanceBuffers& operator = (const StaticInstanceBuffers&) = delete;
public:
    StaticInstanceBuffers(
        std::vector<TransformationAndBillboardId>&& instances,
        uint32_t num_billboard_atlas_components,
        const std::string& name);
    virtual ~StaticInstanceBuffers() override;
    virtual void bind_position_yangles(GLuint attribute_index) const override;
    virtual void bind_position(GLuint attribute_index) const override;
    virtual void bind_billboard_atlas_instances(GLuint attribute_index) const override;
    virtual GLsizei num_instances() const override;
private:
    std::vector<TransformationAndBillboardId> instances_;
    StaticPositionYAngles position_yangles_;
    StaticPosition position_;
    StaticBillboardIds billboard_ids_;
};

}
