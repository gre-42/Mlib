#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Render/Renderables/Renderable_Colored_Vertex_Array.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>

namespace Mlib {

class RenderableBlendingX: public SceneNodeResource {
public:
    RenderableBlendingX(
        const FixedArray<float, 2, 2>& square,
        const std::string& texture,
        RenderingResources* rendering_resources);
    virtual void instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter) override;
private:
    FixedArray<std::shared_ptr<RenderableColoredVertexArray>, 2> rva_;
    // Square is stored to facilitate creating depth-sorted nodes during instantiation.
    FixedArray<float, 2, 2> square_;

};

}
