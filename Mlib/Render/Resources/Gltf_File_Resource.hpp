#pragma once
#include <Mlib/Render/Heterogeneous_Resource_Instantiator.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct LoadMeshConfig;
class ColoredVertexArrayResource;
class RenderingResources;

class GltfFileResource: public SceneNodeResource {
public:
    GltfFileResource(
        const std::string& filename,
        const LoadMeshConfig& cfg,
        const SceneNodeResources& scene_node_resources);
    virtual ~GltfFileResource() override;

    // SceneNodeResource, Misc
    virtual void instantiate_renderable(const std::string& name, SceneNode& scene_node, const RenderableResourceFilter& renderable_resource_filter) const override;
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
private:
    HeterogeneousResourceInstantiator hri_;
    std::shared_ptr<ColoredVertexArrayResource> rva_;
};

}
