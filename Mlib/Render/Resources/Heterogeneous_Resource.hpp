#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Threads/Safe_Recursive_Shared_Mutex.hpp>
#include <memory>
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
        float instance_scale = 1.f,
        const TransformationMatrix<double, double, 3>& geographic_mapping = TransformationMatrix<double, double, 3>::identity());
    virtual ~HeterogeneousResource() override;
    
    // ISceneNodeResource, Misc
    virtual void preload(const RenderableResourceFilter& filter) const override;
    virtual void instantiate_child_renderable(const ChildInstantiationOptions& options) const override;
    virtual void instantiate_root_renderables(const RootInstantiationOptions& options) const override;
    virtual TransformationMatrix<double, double, 3> get_geographic_mapping(const TransformationMatrix<double, double, 3>& absolute_model_matrix) const override;
    virtual AggregateMode get_aggregate_mode() const override;
    virtual void print(std::ostream& ostr) const override;

    // ISceneNodeResource, Animation
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_physics_arrays() const override;
    virtual std::list<std::shared_ptr<AnimatedColoredVertexArrays>> get_rendering_arrays() const override;
    virtual std::list<TypedMesh<std::shared_ptr<IIntersectable>>> get_intersectables() const override;
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
    virtual void smoothen_edges(
        SmoothnessTarget target,
        float smoothness,
        size_t niterations,
        float decay = 0.97f) override;

    // ISceneNodeResource, Transformations
    virtual std::shared_ptr<ISceneNodeResource> generate_grind_lines(
        float edge_angle,
        float averaged_normal_angle,
        const ColoredVertexArrayFilter& filter) const override;
    virtual std::shared_ptr<ISceneNodeResource> generate_contour_edges() const override;
    virtual void create_barrier_triangle_hitboxes(
        float depth,
        PhysicsMaterial destination_physics_material,
        const ColoredVertexArrayFilter& filter) override;

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
    mutable std::shared_ptr<AnimatedColoredVertexArrays> physics_arrays_;
    mutable SafeAtomicRecursiveSharedMutex rcva_mutex_;
    mutable SafeAtomicRecursiveSharedMutex physics_arrays_mutex_;
    const SceneNodeResources& scene_node_resources_;
    TransformationMatrix<double, double, 3> geographic_mapping_;
};

}
