#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>
#include <functional>
#include <list>
#include <map>
#include <mutex>

namespace Mlib {

struct RenderProgramIdentifier;
struct ColoredRenderProgram;
struct Light;
class SubstitutionInfo;
class RenderingResources;
struct TransformationAndBillboardId;
struct BlendMapTexture;

class ColoredVertexArrayResource:
    public SceneNodeResource,
    public std::enable_shared_from_this<ColoredVertexArrayResource>
{
    friend class RenderableColoredVertexArray;
public:
    typedef std::map<const ColoredVertexArray*, std::vector<TransformationAndBillboardId>> Instances;
    ColoredVertexArrayResource(const ColoredVertexArrayResource& other) = delete;
    ColoredVertexArrayResource& operator = (const ColoredVertexArrayResource& other) = delete;
    ColoredVertexArrayResource(
        const std::shared_ptr<AnimatedColoredVertexArrays>& triangles,
        std::unique_ptr<Instances>&& instances);
    ColoredVertexArrayResource(
        const std::list<std::shared_ptr<ColoredVertexArray>>& triangles,
        std::unique_ptr<Instances>&& instances);
    ColoredVertexArrayResource(
        const std::shared_ptr<ColoredVertexArray>& triangles,
        std::unique_ptr<Instances>&& instances);
    
    explicit ColoredVertexArrayResource(const std::shared_ptr<AnimatedColoredVertexArrays>& triangles);
    explicit ColoredVertexArrayResource(const std::list<std::shared_ptr<ColoredVertexArray>>& triangles);
    explicit ColoredVertexArrayResource(const std::shared_ptr<ColoredVertexArray>& triangles);

    ~ColoredVertexArrayResource();

    // SceneNodeResource, Misc
    virtual void instantiate_renderable(const std::string& name, SceneNode& scene_node, const RenderableResourceFilter& renderable_resource_filter) const override;
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_animated_arrays() const override;
    virtual AggregateMode aggregate_mode() const override;
    virtual void print(std::ostream& ostr) const override;

    // SceneNodeResource, Animation
    virtual void set_absolute_joint_poses(const std::vector<OffsetAndQuaternion<float>>& poses);
    virtual void import_bone_weights(
        const AnimatedColoredVertexArrays& other_acvas,
        float max_distance) override;

    // SceneNodeResource, Modifiers
    virtual void generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles = false) override;
    virtual void generate_ray(const FixedArray<float, 3>& from, const FixedArray<float, 3>& to) override;
    virtual void downsample(size_t factor) override;

    // SceneNodeResource, Transformations
    virtual std::shared_ptr<SceneNodeResource> generate_grind_lines(float edge_angle, float normal_angle) const override;
    virtual std::shared_ptr<SceneNodeResource> generate_contour_edges() const override;

    // SceneNodeResource, Extractions
    virtual std::shared_ptr<SceneNodeResource> extract_alignment_planes(const std::string& object_prefix) override;
    virtual std::shared_ptr<SceneNodeResource> copy_physics_resources(const PhysicsResourceFilter& physics_resource_filter) override;
    virtual std::shared_ptr<SceneNodeResource> copy_renderable_resources(const RenderableResourceFilter& renderable_resource_filter) override;
private:
    const ColoredRenderProgram& get_render_program(
        const RenderProgramIdentifier& id,
        const std::vector<std::pair<TransformationMatrix<float, 3>, Light*>>& filtered_lights,
        const std::vector<size_t>& light_noshadow_indices,
        const std::vector<size_t>& light_shadow_indices,
        const std::vector<size_t>& black_shadow_indices,
        const std::vector<BlendMapTexture*>& textures) const;
    std::shared_ptr<SceneNodeResource> extract_by_predicate(const std::function<bool(const ColoredVertexArray& cva)>& predicate);
    std::shared_ptr<SceneNodeResource> copy_by_predicate(const std::function<bool(const ColoredVertexArray& cva)>& predicate);
    const SubstitutionInfo& get_vertex_array(const std::shared_ptr<ColoredVertexArray>& cva) const;
    std::shared_ptr<AnimatedColoredVertexArrays> triangles_res_;
    mutable std::map<const ColoredVertexArray*, std::unique_ptr<SubstitutionInfo>> vertex_arrays_;
    std::shared_ptr<RenderingResources> rendering_resources_;
    mutable std::mutex mutex_;
    std::unique_ptr<Instances> instances_;
    mutable bool textures_preloaded_;
};

}
