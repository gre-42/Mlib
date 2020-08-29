#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Render/Renderables/Renderable_Colored_Vertex_Array.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>

namespace Mlib {

class RenderableBinaryX:
    public SceneNodeResource,
    public std::enable_shared_from_this<RenderableBinaryX>
{
public:
    RenderableBinaryX(
        const FixedArray<float, 2, 2>& square,
        const std::string& texture,
        RenderingResources* rendering_resources,
        bool is_small);
    virtual void initialize() override;
    virtual void instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter) override;
private:
    std::shared_ptr<RenderableColoredVertexArray> rva_;

};

}
