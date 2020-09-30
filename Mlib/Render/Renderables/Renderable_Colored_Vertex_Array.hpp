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

class RenderingResources;
struct Light;

struct ColoredRenderProgram: public RenderProgram {
    GLint mvp_location;
    std::map<size_t, GLint> mvp_light_locations;
    GLint mvp_dirtmap_location;
    GLint m_location;
    std::map<size_t, GLint> light_dir_locations;
    // GLint light_pos;
    GLint view_pos;
    std::map<size_t, GLint> light_ambiences;
    std::map<size_t, GLint> light_diffusivities;
    std::map<size_t, GLint> light_specularities;
    GLint texture1_location;
    std::map<size_t, GLint> texture_lightmap_color_locations;
    std::map<size_t, GLint> texture_lightmap_depth_locations;
    GLint texture_dirtmap_location;
    GLint texture_dirt_location;
};

struct RenderProgramIdentifier {
    AggregateMode aggregate_mode;
    OccluderType occluder_type;
    size_t nlights;
    BlendMode blend_mode;
    bool has_texture;
    bool has_lightmap_color;
    bool has_lightmap_depth;
    bool has_dirtmap;
    bool has_instances;
    bool reorient_normals;
    bool calculate_lightmap;
    OrderableFixedArray<float, 3> ambience;
    OrderableFixedArray<float, 3> diffusivity;
    OrderableFixedArray<float, 3> specularity;
    std::strong_ordering operator <=> (const RenderProgramIdentifier&) const = default;
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
        const std::list<std::shared_ptr<ColoredVertexArray>>& triangles,
        std::map<const ColoredVertexArray*, std::vector<FixedArray<float, 4, 4>>>* instances,
        RenderingResources* rendering_resources);
    explicit RenderableColoredVertexArray(
        const std::shared_ptr<ColoredVertexArray>& triangles,
        std::map<const ColoredVertexArray*, std::vector<FixedArray<float, 4, 4>>>* instances,
        RenderingResources* rendering_resources);
    virtual void instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter) override;
    virtual std::list<std::shared_ptr<ColoredVertexArray>> get_triangle_meshes() override;
    virtual void generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles = false) override;
    virtual void generate_ray(const FixedArray<float, 3>& from, const FixedArray<float, 3>& to) override;
private:
    const ColoredRenderProgram& get_render_program(
        const RenderProgramIdentifier& id,
        const std::list<std::pair<FixedArray<float, 4, 4>, Light*>>& filtered_lights,
        const std::vector<size_t>& light_noshadow_indices,
        const std::vector<size_t>& light_shadow_indices,
        const std::vector<size_t>& black_shadow_indices) const;
    const VertexArray& get_vertex_array(const ColoredVertexArray* cva) const;
    std::list<std::shared_ptr<ColoredVertexArray>> triangles_res_;
    mutable std::map<RenderProgramIdentifier, std::unique_ptr<ColoredRenderProgram>> render_programs_;
    mutable std::map<const ColoredVertexArray*, std::unique_ptr<VertexArray>> vertex_arrays_;
    RenderingResources* rendering_resources_;
    bool render_textures_;
    mutable std::mutex mutex_;
    std::unique_ptr<std::map<const ColoredVertexArray*, std::vector<FixedArray<float, 4, 4>>>> instances_;
};

}
