#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Threads/Safe_Shared_Mutex.hpp>
#include <functional>
#include <list>
#include <map>

namespace Mlib {

struct RenderProgramIdentifier;
struct ColoredRenderProgram;
struct Light;
class SubstitutionInfo;
class SceneNodeResources;
class RenderingResources;
struct TransformationAndBillboardId;
struct BlendMapTexture;
class IInstanceBuffers;

class ColoredVertexArrayResource:
    public ISceneNodeResource,
    public std::enable_shared_from_this<ColoredVertexArrayResource>
{
    friend class RenderableColoredVertexArray;

    ColoredVertexArrayResource(const ColoredVertexArrayResource& other) = delete;
    ColoredVertexArrayResource& operator = (const ColoredVertexArrayResource& other) = delete;
public:
    using Instances = std::map<const ColoredVertexArray<float>*, std::shared_ptr<IInstanceBuffers>>;
    ColoredVertexArrayResource(
        std::shared_ptr<AnimatedColoredVertexArrays> triangles,
        std::unique_ptr<Instances>&& instances);
    ColoredVertexArrayResource(
        const std::list<std::shared_ptr<ColoredVertexArray<float>>>& striangles,
        const std::list<std::shared_ptr<ColoredVertexArray<double>>>& dtriangles,
        std::unique_ptr<Instances>&& instances);
    ColoredVertexArrayResource(
        const std::shared_ptr<ColoredVertexArray<float>>& striangles,
        std::unique_ptr<Instances>&& instances);
    ColoredVertexArrayResource(
        const std::shared_ptr<ColoredVertexArray<double>>& dtriangles,
        std::unique_ptr<Instances>&& instances);
    ColoredVertexArrayResource(
        const std::shared_ptr<ColoredVertexArray<float>>& striangles,
        const std::shared_ptr<IInstanceBuffers>& instances);
    
    explicit ColoredVertexArrayResource(const std::shared_ptr<AnimatedColoredVertexArrays>& triangles);
    ColoredVertexArrayResource(
        const std::list<std::shared_ptr<ColoredVertexArray<float>>>& striangles);
    ColoredVertexArrayResource(
        const std::list<std::shared_ptr<ColoredVertexArray<double>>>& striangles);
    ColoredVertexArrayResource(
        const std::list<std::shared_ptr<ColoredVertexArray<float>>>& striangles,
        const std::list<std::shared_ptr<ColoredVertexArray<double>>>& dtriangles);
    ColoredVertexArrayResource(
        const std::shared_ptr<ColoredVertexArray<float>>& striangles);
    ColoredVertexArrayResource(
        const std::shared_ptr<ColoredVertexArray<double>>& striangles);

    ~ColoredVertexArrayResource();

    // ISceneNodeResource, Misc
    virtual void preload(const RenderableResourceFilter& filter) const override;
    virtual void instantiate_renderable(const InstantiationOptions& options) const override;
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_physics_arrays() const override;
    virtual AggregateMode aggregate_mode() const override;
    virtual void print(std::ostream& ostr) const override;

    // ISceneNodeResource, Animation
    virtual void set_absolute_joint_poses(const std::vector<OffsetAndQuaternion<float, float>>& poses);
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
    const ColoredRenderProgram& get_render_program(
        const RenderProgramIdentifier& id,
        const std::vector<std::pair<TransformationMatrix<float, double, 3>, Light*>>& filtered_lights,
        const std::vector<size_t>& lightmap_indices,
        const std::vector<size_t>& light_noshadow_indices,
        const std::vector<size_t>& light_shadow_indices,
        const std::vector<size_t>& black_shadow_indices,
        const std::vector<BlendMapTexture*>& textures_color,
        const std::vector<BlendMapTexture*>& textures_alpha) const;
    void deallocate();
    bool requires_aggregation(const ColoredVertexArray<float> &cva) const;
    const SubstitutionInfo& get_vertex_array(const std::shared_ptr<ColoredVertexArray<float>>& cva) const;
    std::shared_ptr<AnimatedColoredVertexArrays> triangles_res_;
    mutable std::map<const ColoredVertexArray<float>*, std::unique_ptr<SubstitutionInfo>> vertex_arrays_;
    SceneNodeResources& scene_node_resources_;
    std::shared_ptr<RenderingResources> rendering_resources_;
    mutable SafeSharedMutex mutex_;
    std::unique_ptr<Instances> instances_;
    DeallocationToken deallocation_token_;
};

}
