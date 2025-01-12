#pragma once
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>

namespace Mlib {

struct Material;
struct Morphology;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
class ColoredVertexArrayResource;

class GridResource: public ISceneNodeResource {
public:
    GridResource(
        const FixedArray<size_t, 2>& size,
        const TransformationMatrix<float, double, 3>& transformation,
        double tile_length,
        double scale,
        double uv_scale,
        double period,
        const Material& material,
        const Morphology& morphology);

    // ISceneNodeResource, Misc
    virtual void preload(const RenderableResourceFilter& filter) const override;
    virtual void instantiate_child_renderable(const ChildInstantiationOptions& options) const override;
    virtual void instantiate_root_renderables(const RootInstantiationOptions& options) const override;
    virtual AggregateMode get_aggregate_mode() const override;
    virtual std::list<SpawnPoint> get_spawn_points() const override;
    virtual WayPointSandboxes get_way_points() const override;

    // ISceneNodeResource, Animation
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_physics_arrays() const override;

    // ISceneNodeResource, Modifiers
    virtual void generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles = false) override;
    virtual void modify_physics_material_tags(
        PhysicsMaterial add,
        PhysicsMaterial remove,
        const ColoredVertexArrayFilter& filter) override;
    virtual void generate_instances() override;
    virtual void create_barrier_triangle_hitboxes(
        float depth,
        PhysicsMaterial destination_physics_material,
        const ColoredVertexArrayFilter& filter) override;

private:
    std::shared_ptr<ColoredVertexArrayResource> rva_;

};

}
