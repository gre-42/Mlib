#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>

namespace Mlib {

struct Material;

class BinaryXResource: public SceneNodeResource {
public:
    BinaryXResource(
        const FixedArray<float, 2, 2>& square,
        const Material& material);
    virtual void instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter) const override;
    virtual AggregateMode aggregate_mode() const override;
private:
    std::shared_ptr<ColoredVertexArrayResource> rva_;

};

}
