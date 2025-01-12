#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Map/Unordered_Map.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Threads/Safe_Recursive_Shared_Mutex.hpp>
#include <functional>
#include <list>
#include <unordered_map>

namespace Mlib {

struct AttributeIndexCalculator;
struct RenderProgramIdentifier;
struct ColoredRenderProgram;
struct Light;
struct Skidmark;
class SceneNodeResources;
class RenderingResources;
struct TransformationAndBillboardId;
struct BlendMapTexture;
class IVertexData;
class IInstanceBuffers;
enum class TaskLocation;
struct BlendMapTextureAndId;

class ColoredVertexArrayResource:
    public ISceneNodeResource,
    public std::enable_shared_from_this<ColoredVertexArrayResource>
{
    friend class RenderableColoredVertexArray;

    ColoredVertexArrayResource(const ColoredVertexArrayResource& other) = delete;
    ColoredVertexArrayResource& operator = (const ColoredVertexArrayResource& other) = delete;
public:
    using Vertices = UnorderedMap<const ColoredVertexArray<float>*, std::shared_ptr<IVertexData>>;
    using Instances = UnorderedMap<const ColoredVertexArray<float>*, std::shared_ptr<IInstanceBuffers>>;
    ColoredVertexArrayResource(
        std::shared_ptr<AnimatedColoredVertexArrays> triangles,
        Vertices&& vertices,
        std::unique_ptr<Instances>&& instances,
        std::weak_ptr<ColoredVertexArrayResource> vertex_data);
    ColoredVertexArrayResource(
        const std::list<std::shared_ptr<ColoredVertexArray<float>>>& striangles,
        const std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& dtriangles,
        Vertices&& vertices,
        std::unique_ptr<Instances>&& instances,
        std::weak_ptr<ColoredVertexArrayResource> vertex_data);
    ColoredVertexArrayResource(
        const std::shared_ptr<ColoredVertexArray<float>>& striangles,
        Vertices&& vertices,
        std::unique_ptr<Instances>&& instances,
        std::weak_ptr<ColoredVertexArrayResource> vertex_data);
    ColoredVertexArrayResource(
        const std::shared_ptr<ColoredVertexArray<CompressedScenePos>>& dtriangles,
        std::unique_ptr<Instances>&& instances);
    ColoredVertexArrayResource(
        const std::shared_ptr<ColoredVertexArray<float>>& striangles,
        const std::shared_ptr<IInstanceBuffers>& instances);
    ColoredVertexArrayResource(
        const std::shared_ptr<ColoredVertexArray<float>>& striangles,
        const std::shared_ptr<IVertexData>& vertices);
    
    explicit ColoredVertexArrayResource(const std::shared_ptr<AnimatedColoredVertexArrays>& triangles);
    ColoredVertexArrayResource(
        const std::list<std::shared_ptr<ColoredVertexArray<float>>>& striangles);
    ColoredVertexArrayResource(
        const std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& striangles);
    ColoredVertexArrayResource(
        const std::list<std::shared_ptr<ColoredVertexArray<float>>>& striangles,
        const std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& dtriangles);
    ColoredVertexArrayResource(
        const std::shared_ptr<ColoredVertexArray<float>>& striangles);
    ColoredVertexArrayResource(
        const std::shared_ptr<ColoredVertexArray<CompressedScenePos>>& striangles);

    ~ColoredVertexArrayResource();

    // ISceneNodeResource, Misc
    virtual void preload(const RenderableResourceFilter& filter) const override;
    virtual void instantiate_child_renderable(const ChildInstantiationOptions& options) const override;
    virtual void instantiate_root_renderables(const RootInstantiationOptions& options) const override;
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_physics_arrays() const override;
    virtual AggregateMode get_aggregate_mode() const override;
    virtual void print(std::ostream& ostr) const override;

    // ISceneNodeResource, Animation
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
    
    bool copy_in_progress() const;
    void wait() const;
private:
    AttributeIndexCalculator get_attribute_index_calculator(const ColoredVertexArray<float>& cva) const;
    const ColoredRenderProgram& get_render_program(
        const RenderProgramIdentifier& id,
        const ColoredVertexArray<float>& cva,
        const std::vector<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>>& filtered_lights,
        const std::vector<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>>& filtered_skidmarks,
        const std::vector<size_t>& lightmap_indices,
        const std::vector<size_t>& light_noshadow_indices,
        const std::vector<size_t>& light_shadow_indices,
        const std::vector<size_t>& black_shadow_indices,
        const std::vector<BlendMapTextureAndId>& textures_color,
        const std::vector<BlendMapTextureAndId>& textures_alpha) const;
    bool requires_aggregation(const ColoredVertexArray<float> &cva) const;
    IVertexData& get_vertex_array(
        const std::shared_ptr<ColoredVertexArray<float>>& cva,
        TaskLocation task_location) const;
    std::shared_ptr<AnimatedColoredVertexArrays> triangles_res_;
    SceneNodeResources& scene_node_resources_;
    RenderingResources& rendering_resources_;
    mutable Vertices vertex_arrays_;
    std::unique_ptr<Instances> instances_;
    std::weak_ptr<ColoredVertexArrayResource> vertex_data_;
    mutable SafeAtomicRecursiveSharedMutex mutex_;
};

}
