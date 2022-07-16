#pragma once
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>

namespace Mlib {

struct Material;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
class ColoredVertexArrayResource;

class GridResource: public SceneNodeResource {
public:
    GridResource(
        const FixedArray<size_t, 2>& size,
        const TransformationMatrix<float, double, 3>& transformation,
        double scale,
        double uv_scale,
        double period,
        const Material& material);

    // SceneNodeResource, Misc
    virtual void instantiate_renderable(const std::string& name, SceneNode& scene_node, const RenderableResourceFilter& renderable_resource_filter) const override;
    virtual AggregateMode aggregate_mode() const override;

    // SceneNodeResource, Animation
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_animated_arrays() const override;

    // SceneNodeResource, Modifiers
    virtual void generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles = false) override;
    virtual void modify_physics_material_tags(
        PhysicsMaterial add,
        PhysicsMaterial remove,
        const ColoredVertexArrayFilter& filter) override;
    virtual void generate_instances() override;

private:
    std::shared_ptr<ColoredVertexArrayResource> rva_;

};

}
