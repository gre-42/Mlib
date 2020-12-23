#include "Renderable_Obj_File.hpp"
#include <Mlib/Geometry/Mesh/Load_Obj.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>

using namespace Mlib;

RenderableObjFile::RenderableObjFile(
    const std::string& filename,
    const FixedArray<float, 3>& position,
    const FixedArray<float, 3>& rotation,
    const FixedArray<float, 3>& scale,
    RenderingResources& rendering_resources,
    bool is_small,
    BlendMode blend_mode,
    bool cull_faces,
    OccludedType occluded_type,
    OccluderType occluder_type,
    bool occluded_by_black,
    AggregateMode aggregate_mode,
    TransformationMode transformation_mode,
    bool apply_static_lighting,
    bool werror)
{
    std::list<std::shared_ptr<ColoredVertexArray>> triangles = load_obj(
        filename,
        is_small,
        blend_mode,
        cull_faces,
        occluded_type,
        occluder_type,
        occluded_by_black,
        aggregate_mode,
        transformation_mode,
        apply_static_lighting,
        werror);
    FixedArray<float, 3, 3> rotation_matrix{tait_bryan_angles_2_matrix(rotation)};
    for(auto& l : triangles) {
        for(auto& t : l->triangles) {
            for(auto& v : t.flat_iterable()) {
                v.position *= scale;
                v.position = dot1d(rotation_matrix, v.position);
                v.position += position;
                v.normal = dot1d(rotation_matrix, v.normal);
            }
        }
    }
    rva_ = std::make_shared<RenderableColoredVertexArray>(triangles, nullptr, rendering_resources);
}

void RenderableObjFile::instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter) const
{
    rva_->instantiate_renderable(name, scene_node, resource_filter);
}

std::list<std::shared_ptr<ColoredVertexArray>> RenderableObjFile::get_triangle_meshes() const {
    return rva_->get_triangle_meshes();
}

void RenderableObjFile::generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) {
    return rva_->generate_triangle_rays(npoints, lengths, delete_triangles);
}

void RenderableObjFile::generate_ray(const FixedArray<float, 3>& from, const FixedArray<float, 3>& to) {
    return rva_->generate_ray(from, to);
}

AggregateMode RenderableObjFile::aggregate_mode() const {
    return rva_->aggregate_mode();
}