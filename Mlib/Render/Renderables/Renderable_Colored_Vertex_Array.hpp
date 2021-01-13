#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Instance_Handles/Vertex_Array.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>
#include <list>
#include <map>
#include <mutex>

namespace Mlib {

struct RenderProgramIdentifier;
struct ColoredRenderProgram;
struct Light;
class RenderingResources;

struct SubstitutionInfo {
    VertexArray va;
    std::shared_ptr<ColoredVertexArray> cva;
    size_t ntriangles;
    size_t nlines;
    std::vector<size_t> triangles_local_ids;
    std::vector<size_t> triangles_global_ids;
    size_t current_triangle_id = SIZE_MAX;
    void delete_triangle(size_t id, FixedArray<ColoredVertex, 3>* ptr);
    void insert_triangle(size_t id, FixedArray<ColoredVertex, 3>* ptr);
    void delete_triangles_far_away(
        const FixedArray<float, 3>& position,
        const TransformationMatrix<float>& m,
        float distance_add,
        float distance_remove,
        size_t noperations);
};

class RenderableColoredVertexArray:
    public SceneNodeResource,
    public std::enable_shared_from_this<RenderableColoredVertexArray>
{
    friend class RenderableColoredVertexArrayInstance;
public:
    RenderableColoredVertexArray(const RenderableColoredVertexArray& other) = delete;
    RenderableColoredVertexArray& operator = (const RenderableColoredVertexArray& other) = delete;
    explicit RenderableColoredVertexArray(
        const std::shared_ptr<AnimatedColoredVertexArrays>& triangles,
        std::map<const ColoredVertexArray*, std::vector<TransformationMatrix<float>>>* instances,
        RenderingResources& rendering_resources);
    explicit RenderableColoredVertexArray(
        const std::list<std::shared_ptr<ColoredVertexArray>>& triangles,
        std::map<const ColoredVertexArray*, std::vector<TransformationMatrix<float>>>* instances,
        RenderingResources& rendering_resources);
    explicit RenderableColoredVertexArray(
        const std::shared_ptr<ColoredVertexArray>& triangles,
        std::map<const ColoredVertexArray*, std::vector<TransformationMatrix<float>>>* instances,
        RenderingResources& rendering_resources);
    ~RenderableColoredVertexArray();
    virtual void instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter) const override;
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_animated_arrays() const override;
    virtual void generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles = false) override;
    virtual void generate_ray(const FixedArray<float, 3>& from, const FixedArray<float, 3>& to) override;
    virtual AggregateMode aggregate_mode() const override;
    virtual void set_absolute_joint_poses(const std::vector<OffsetAndQuaternion<float>>& poses);
    virtual void downsample(size_t factor);
    virtual void import_bone_weights(
        const AnimatedColoredVertexArrays& other_acvas,
        float max_distance) override;
private:
    const ColoredRenderProgram& get_render_program(
        const RenderProgramIdentifier& id,
        const std::vector<std::pair<TransformationMatrix<float>, Light*>>& filtered_lights,
        const std::vector<size_t>& light_noshadow_indices,
        const std::vector<size_t>& light_shadow_indices,
        const std::vector<size_t>& black_shadow_indices) const;
    const SubstitutionInfo& get_vertex_array(const std::shared_ptr<ColoredVertexArray>& cva) const;
    std::shared_ptr<AnimatedColoredVertexArrays> triangles_res_;
    mutable std::map<const ColoredVertexArray*, std::unique_ptr<SubstitutionInfo>> vertex_arrays_;
    RenderingResources& rendering_resources_;
    mutable std::mutex mutex_;
    std::unique_ptr<std::map<const ColoredVertexArray*, std::vector<TransformationMatrix<float>>>> instances_;
};

}
