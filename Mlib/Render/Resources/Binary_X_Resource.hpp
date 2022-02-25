#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>

namespace Mlib {

struct Material;
class ColoredVertexArrayResource;

class BinaryXResource: public SceneNodeResource {
public:
    BinaryXResource(
        const FixedArray<float, 2, 2>& square,
        const Material& material_0,
        const Material& material_90);
    ~BinaryXResource();
    virtual void instantiate_renderable(const std::string& name, SceneNode& scene_node, const RenderableResourceFilter& renderable_resource_filter) const override;
    virtual AggregateMode aggregate_mode() const override;
private:
    std::shared_ptr<ColoredVertexArrayResource> rva_0_;
    std::shared_ptr<ColoredVertexArrayResource> rva_90_;

};

}
