#include "Renderable_Mhx2_File.hpp"
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Load_Mhx2.hpp>

using namespace Mlib;

RenderableMhx2File::RenderableMhx2File(
    const std::string& filename,
    const LoadMeshConfig& cfg,
    RenderingResources& rendering_resources)
{
    std::shared_ptr<AnimatedColoredVertexArrays> acvas = load_mhx2(filename, cfg);
    rva_ = std::make_shared<RenderableColoredVertexArray>(acvas->cvas, nullptr, rendering_resources);
}

void RenderableMhx2File::instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter) const
{
    rva_->instantiate_renderable(name, scene_node, resource_filter);
}

std::list<std::shared_ptr<ColoredVertexArray>> RenderableMhx2File::get_triangle_meshes() const {
    return rva_->get_triangle_meshes();
}

void RenderableMhx2File::generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) {
    return rva_->generate_triangle_rays(npoints, lengths, delete_triangles);
}

void RenderableMhx2File::generate_ray(const FixedArray<float, 3>& from, const FixedArray<float, 3>& to) {
    return rva_->generate_ray(from, to);
}

AggregateMode RenderableMhx2File::aggregate_mode() const {
    return rva_->aggregate_mode();
}