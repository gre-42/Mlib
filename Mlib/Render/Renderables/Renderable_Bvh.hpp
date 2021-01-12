#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>
#include <list>

namespace Mlib {

struct Material;
struct ColoredVertex;
class RenderingResources;

class RenderableBvh: public SceneNodeResource {
public:
    RenderableBvh(
        const std::list<std::shared_ptr<ColoredVertexArray>>& cvas,
        RenderingResources& rendering_resources);
    virtual void instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter) const override;
private:
    std::list<std::shared_ptr<ColoredVertexArray>> cvas_;
    Bvh<float, std::pair<const Material*, const FixedArray<ColoredVertex, 3>*>, 3> bvh_;
    RenderingResources& rendering_resources_;
};

}
