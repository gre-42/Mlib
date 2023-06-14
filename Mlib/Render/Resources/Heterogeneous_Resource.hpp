#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
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

class HeterogeneousResource: public ISceneNodeResource {
public:
    explicit HeterogeneousResource(
        const SceneNodeResources& scene_node_resources,
        const FixedArray<float, 3>& instance_rotation = fixed_zeros<float, 3>(),
        float instance_scale = 1.f);
    virtual ~HeterogeneousResource() override;
    
    // ISceneNodeResource, Misc
    virtual void preload() const override;
    virtual void instantiate_renderable(const InstantiationOptions& options) const override;
    virtual AggregateMode aggregate_mode() const override;
    virtual void print(std::ostream& ostr) const override;

    // ISceneNodeResource, Animation
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_animated_arrays() const override;
    virtual void import_bone_weights(
        const AnimatedColoredVertexArrays& other_acvas,
        float max_distance) override;

    // ISceneNodeResource, Modifiers
    virtual void generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles = false) override;
    virtual void generate_ray(const FixedArray<float, 3>& from, const FixedArray<float, 3>& to) override;
    virtual void downsample(size_t n) override;
    virtual void modify_physics_material_tags(
        PhysicsMaterial add,
        PhysicsMaterial remove,
        const ColoredVertexArrayFilter& filter) override;
    virtual void generate_instances() override;

    // ISceneNodeResource, Transformations
    virtual std::shared_ptr<ISceneNodeResource> generate_grind_lines(
        float edge_angle,
        float averaged_normal_angle,
        const ColoredVertexArrayFilter& filter) const override;
    virtual std::shared_ptr<ISceneNodeResource> generate_contour_edges() const override;
    virtual void convex_decompose_terrain(
        const FixedArray<double, 3>& shift,
        PhysicsMaterial destination_physics_material,
        const ColoredVertexArrayFilter& filter) const override;

    // Cereal
    template <class Archive>
    void serialize(Archive& archive) {
        archive(bri);
        archive(acvas);
    }

    // Custom
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
