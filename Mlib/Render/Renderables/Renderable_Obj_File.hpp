#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Render/Renderables/Renderable_Colored_Vertex_Array.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>

namespace Mlib {

struct LoadMeshConfig;

class RenderableObjFile: public SceneNodeResource {
public:
    RenderableObjFile(
        const std::string& filename,
        const LoadMeshConfig& cfg,
        RenderingResources& rendering_resources);
    virtual void instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter) const override;
    virtual std::list<std::shared_ptr<ColoredVertexArray>> get_triangle_meshes() const override;
    virtual void generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles = false) override;
    virtual void generate_ray(const FixedArray<float, 3>& from, const FixedArray<float, 3>& to) override;
    virtual AggregateMode aggregate_mode() const override;
private:
    std::shared_ptr<RenderableColoredVertexArray> rva_;

};

}
