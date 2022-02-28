#pragma once
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>
#include <memory>
#include <mutex>
#include <string>

namespace Mlib {

class SceneNode;
struct RenderableResourceFilter;
struct AnimatedColoredVertexArrays;
class BatchResourceInstantiator;
class ColoredVertexArrayResource;
class SceneNodeResources;

class HeterogeneousResourceInstantiator {
public:
    explicit HeterogeneousResourceInstantiator(
        const SceneNodeResources& scene_node_resources);
    ~HeterogeneousResourceInstantiator();
    void instantiate_renderable(
        const std::string& name,
        SceneNode& scene_node,
        const FixedArray<float, 3>& rotation,
        float scale,
        const RenderableResourceFilter& renderable_resource_filter) const;
    std::shared_ptr<AnimatedColoredVertexArrays> get_animated_arrays(
        float scale) const;
    void generate_instances();
    std::unique_ptr<BatchResourceInstantiator> bri;
    std::shared_ptr<AnimatedColoredVertexArrays> acvas;
private:
    mutable std::shared_ptr<ColoredVertexArrayResource> rcva_;
    mutable std::shared_ptr<AnimatedColoredVertexArrays> acvas_;
    mutable std::mutex rcva_mutex_;
    mutable std::mutex acvas_mutex_;
    const SceneNodeResources& scene_node_resources_;
};

}
