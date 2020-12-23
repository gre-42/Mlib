#pragma once
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Instance_Handles/Vertex_Array.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>
#include <list>
#include <map>
#include <mutex>

namespace Mlib {

struct RenderProgramIdentifier;
struct ColoredRenderProgram;
class RenderingResources;
struct Light;

class RenderableColoredVertexArray:
    public SceneNodeResource,
    public std::enable_shared_from_this<RenderableColoredVertexArray>
{
    friend class RenderableColoredVertexArrayInstance;
public:
    RenderableColoredVertexArray(const RenderableColoredVertexArray& other) = delete;
    RenderableColoredVertexArray& operator = (const RenderableColoredVertexArray& other) = delete;
    explicit RenderableColoredVertexArray(
        const std::list<std::shared_ptr<ColoredVertexArray>>& triangles,
        std::map<const ColoredVertexArray*, std::vector<FixedArray<float, 4, 4>>>* instances,
        RenderingResources& rendering_resources);
    explicit RenderableColoredVertexArray(
        const std::shared_ptr<ColoredVertexArray>& triangles,
        std::map<const ColoredVertexArray*, std::vector<FixedArray<float, 4, 4>>>* instances,
        RenderingResources& rendering_resources);
    virtual void instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter) const override;
    virtual std::list<std::shared_ptr<ColoredVertexArray>> get_triangle_meshes() const override;
    virtual void generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles = false) override;
    virtual void generate_ray(const FixedArray<float, 3>& from, const FixedArray<float, 3>& to) override;
    virtual AggregateMode aggregate_mode() const override;
private:
    const ColoredRenderProgram& get_render_program(
        const RenderProgramIdentifier& id,
        const std::list<std::pair<FixedArray<float, 4, 4>, Light*>>& filtered_lights,
        const std::vector<size_t>& light_noshadow_indices,
        const std::vector<size_t>& light_shadow_indices,
        const std::vector<size_t>& black_shadow_indices) const;
    const VertexArray& get_vertex_array(const ColoredVertexArray* cva) const;
    std::list<std::shared_ptr<ColoredVertexArray>> triangles_res_;
    mutable std::map<const ColoredVertexArray*, std::unique_ptr<VertexArray>> vertex_arrays_;
    RenderingResources& rendering_resources_;
    mutable std::mutex mutex_;
    std::unique_ptr<std::map<const ColoredVertexArray*, std::vector<FixedArray<float, 4, 4>>>> instances_;
};

}
