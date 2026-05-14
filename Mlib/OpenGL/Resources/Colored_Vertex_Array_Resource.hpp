#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Initialization/Default_Uninitialized_Vector.hpp>
#include <Mlib/Map/Threadsafe_Unordered_Map.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <functional>
#include <list>
#include <optional>

namespace Mlib {

class SceneNodeResources;
class RenderingResources;
class IGpuVertexData;
class IGpuInstanceBuffers;
class IGpuObjectFactory;
class IGpuVertexArray;

class ColoredVertexArrayResource:
    public ISceneNodeResource,
    public std::enable_shared_from_this<ColoredVertexArrayResource>
{
    friend class RenderableColoredVertexArray;

    ColoredVertexArrayResource(const ColoredVertexArrayResource& other) = delete;
    ColoredVertexArrayResource& operator = (const ColoredVertexArrayResource& other) = delete;
public:
    #ifndef WITHOUT_GRAPHICS
    using GpuVertices = std::list<std::shared_ptr<IGpuVertexData>>;
    explicit ColoredVertexArrayResource(
        std::list<std::shared_ptr<IGpuVertexArray>> gpu_vertex_arrays);
    explicit ColoredVertexArrayResource(
        std::shared_ptr<IGpuVertexArray> gpu_vertex_array);
    ColoredVertexArrayResource(
        std::shared_ptr<IGpuVertexData> gpu_vertex_data,
        std::shared_ptr<IGpuInstanceBuffers> gpu_instances);
    #endif
    
    explicit ColoredVertexArrayResource(const std::shared_ptr<AnimatedColoredVertexArrays>& triangles);
    ColoredVertexArrayResource(
        const std::list<std::shared_ptr<ColoredVertexArray<float>>>& striangles);
    ColoredVertexArrayResource(
        const std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& striangles);
    ColoredVertexArrayResource(
        const std::list<std::shared_ptr<ColoredVertexArray<float>>>& striangles,
        const std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& dtriangles);
    explicit ColoredVertexArrayResource(
        const std::shared_ptr<ColoredVertexArray<float>>& striangles);
    explicit ColoredVertexArrayResource(
        const std::shared_ptr<ColoredVertexArray<CompressedScenePos>>& striangles);

    ~ColoredVertexArrayResource();

    // ISceneNodeResource, Misc
    virtual void preload(const RenderableResourceFilter& filter) const override;
    virtual void instantiate_child_renderable(const ChildInstantiationOptions& options) const override;
    virtual void instantiate_root_renderables(const RootInstantiationOptions& options) const override;
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_arrays(const ColoredVertexArrayFilter& filter) const override;
    virtual AggregateMode get_aggregate_mode() const override;
    virtual PhysicsMaterial get_physics_material() const override;
    virtual void print(std::ostream& ostr) const override;

    // ISceneNodeResource, Animation
    virtual std::list<std::shared_ptr<AnimatedColoredVertexArrays>> get_rendering_arrays() const override;
    virtual std::list<TypedMesh<std::shared_ptr<IIntersectable>>> get_intersectables() const override;
    void set_absolute_joint_poses(const UUVector<OffsetAndQuaternion<float, float>>& poses);
    virtual void import_bone_weights(
        const AnimatedColoredVertexArrays& other_acvas,
        float max_distance) override;

    // ISceneNodeResource, Modifiers
    virtual void generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles = false) override;
    virtual void generate_ray(const FixedArray<float, 3>& from, const FixedArray<float, 3>& to) override;
    virtual void downsample(size_t factor) override;
    virtual void create_barrier_triangle_hitboxes(
        float depth,
        PhysicsMaterial destination_physics_material,
        const ColoredVertexArrayFilter& filter) override;

    // ISceneNodeResource, Transformations
    virtual std::shared_ptr<ISceneNodeResource> generate_grind_lines(
        float edge_angle,
        float averaged_normal_angle,
        const ColoredVertexArrayFilter& filter) const override;
    virtual std::shared_ptr<ISceneNodeResource> generate_contour_edges() const override;

    // ISceneNodeResource, Extractions
    virtual void modify_physics_material_tags(
        PhysicsMaterial add,
        PhysicsMaterial remove,
        const ColoredVertexArrayFilter& filter) override;
    
    #ifndef WITHOUT_GRAPHICS
    bool copy_in_progress() const;
    void wait() const;
    #endif
private:
    ColoredVertexArrayResource(
        std::shared_ptr<AnimatedColoredVertexArrays> triangles,
        std::list<std::shared_ptr<IGpuVertexData>> gpu_vertex_data,
        std::list<std::shared_ptr<IGpuVertexArray>> gpu_vertex_arrays);
    bool requires_aggregation(const ColoredVertexArray<float> &cva) const;
    SceneNodeResources& scene_node_resources_;
    #ifndef WITHOUT_GRAPHICS
    RenderingResources& rendering_resources_;
    IGpuObjectFactory& gpu_object_factory_;
    std::list<std::shared_ptr<IGpuVertexData>> gpu_vertex_data_;
    std::list<std::shared_ptr<IGpuVertexArray>> gpu_vertex_arrays_;
    #endif
    std::shared_ptr<AnimatedColoredVertexArrays> triangles_res_;
    mutable FastMutex gpu_triangles_res_mutex_;
};

}
