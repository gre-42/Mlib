#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Render/Renderables/Renderable_Colored_Vertex_Array.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>

namespace Mlib {

class RenderableObjFile: public SceneNodeResource {
public:
    RenderableObjFile(
        const std::string& filename,
        const FixedArray<float, 3>& position,
        const FixedArray<float, 3>& rotation,
        const FixedArray<float, 3>& scale,
        RenderingResources* rendering_resources,
        bool is_small,
        BlendMode blend_mode,
        bool blend_cull_faces,
        OccludedType occluded_type,
        OccluderType occluder_type,
        bool occluded_by_black,
        AggregateMode aggregate_mode,
        bool apply_static_lighting,
        bool werror);
    virtual void instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter) const override;
    virtual std::list<std::shared_ptr<ColoredVertexArray>> get_triangle_meshes() const override;
    virtual void generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles = false) override;
    virtual void generate_ray(const FixedArray<float, 3>& from, const FixedArray<float, 3>& to) override;
private:
    std::shared_ptr<RenderableColoredVertexArray> rva_;

};

}
