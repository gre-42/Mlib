#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>

namespace Mlib {

class ColoredVertexArrayResource;

class BlendingXResource: public SceneNodeResource {
public:
    BlendingXResource(
        const FixedArray<float, 2, 2>& square,
        const std::string& texture);
    ~BlendingXResource();
    virtual void instantiate_renderable(const std::string& name, SceneNode& scene_node, const RenderableResourceFilter& renderable_resource_filter) const override;
private:
    FixedArray<std::shared_ptr<ColoredVertexArrayResource>, 2> rva_;
    // Square is stored to facilitate creating depth-sorted nodes during instantiation.
    FixedArray<float, 2, 2> square_;

};

}
