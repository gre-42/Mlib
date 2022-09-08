#pragma once
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>
#include <memory>
#include <shared_mutex>
#include <string>

namespace Mlib {

class SceneNode;
struct RenderableResourceFilter;
struct AnimatedColoredVertexArrays;
class BatchResourceInstantiator;
class ColoredVertexArrayResource;
class SceneNodeResources;
class ISupplyDepots;

class HeterogeneousResource: public SceneNodeResource {
public:
    explicit HeterogeneousResource(
        const SceneNodeResources& scene_node_resources);
    virtual ~HeterogeneousResource() override;
    
    // SceneNodeResource, Misc
    virtual void preload() const override;
    virtual void instantiate_renderable(const InstantiationOptions& options) const override;
    virtual AggregateMode aggregate_mode() const override;
    virtual void print(std::ostream& ostr) const override;

    // SceneNodeResource, Animation
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_animated_arrays() const override;
    virtual void import_bone_weights(
        const AnimatedColoredVertexArrays& other_acvas,
        float max_distance) override;

    // SceneNodeResource, Modifiers
    virtual void generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles = false) override;
    virtual void generate_ray(const FixedArray<float, 3>& from, const FixedArray<float, 3>& to) override;
    virtual void downsample(size_t n) override;
    virtual void modify_physics_material_tags(
        PhysicsMaterial add,
        PhysicsMaterial remove,
        const ColoredVertexArrayFilter& filter) override;
    virtual void generate_instances() override;

    // SceneNodeResource, Transformations
    virtual std::shared_ptr<SceneNodeResource> generate_grind_lines(
        float edge_angle,
        float averaged_normal_angle,
        const ColoredVertexArrayFilter& filter) const override;
    virtual std::shared_ptr<SceneNodeResource> generate_contour_edges() const override;

    // Custom
    void instantiate_renderable(
        const InstantiationOptions& options,
        const FixedArray<float, 3>& rotation,
        float scale) const;
    std::shared_ptr<AnimatedColoredVertexArrays> get_animated_arrays(
        float scale) const;
    std::unique_ptr<BatchResourceInstantiator> bri;
    std::shared_ptr<AnimatedColoredVertexArrays> acvas;
private:
    mutable std::shared_ptr<ColoredVertexArrayResource> rcva_;
    mutable std::shared_ptr<AnimatedColoredVertexArrays> acvas_;
    mutable std::shared_mutex rcva_mutex_;
    mutable std::shared_mutex acvas_mutex_;
    const SceneNodeResources& scene_node_resources_;
};

}
