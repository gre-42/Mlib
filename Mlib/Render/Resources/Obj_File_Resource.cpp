#include "Obj_File_Resource.hpp"
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Load_Obj.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>

using namespace Mlib;

ObjFileResource::ObjFileResource(
    const std::string& filename,
    const LoadMeshConfig& cfg)
{
    acvas_ = std::make_shared<AnimatedColoredVertexArrays>();
    acvas_->cvas = load_obj(filename, cfg);
    rva_ = std::make_shared<ColoredVertexArrayResource>(acvas_, nullptr);
}

ObjFileResource::~ObjFileResource()
{}

void ObjFileResource::instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter) const
{
    rva_->instantiate_renderable(name, scene_node, resource_filter);
}

std::shared_ptr<AnimatedColoredVertexArrays> ObjFileResource::get_animated_arrays() const {
    return rva_->get_animated_arrays();
}

void ObjFileResource::generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) {
    return rva_->generate_triangle_rays(npoints, lengths, delete_triangles);
}

void ObjFileResource::generate_ray(const FixedArray<float, 3>& from, const FixedArray<float, 3>& to) {
    return rva_->generate_ray(from, to);
}

AggregateMode ObjFileResource::aggregate_mode() const {
    return rva_->aggregate_mode();
}

void ObjFileResource::downsample(size_t n) {
    rva_->downsample(n);
}

void ObjFileResource::import_bone_weights(
    const AnimatedColoredVertexArrays& other_acvas,
    float max_distance)
{
    rva_->import_bone_weights(other_acvas, max_distance);
}
