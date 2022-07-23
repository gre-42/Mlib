#include "Obj_File_Resource.hpp"
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Load_Obj.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>

using namespace Mlib;

ObjFileResource::ObjFileResource(
    const std::string& filename,
    const LoadMeshConfig& cfg,
    const SceneNodeResources& scene_node_resources)
: hri_{ scene_node_resources }
{
    hri_.acvas->scvas = load_obj(filename, cfg);
    rva_ = std::make_shared<ColoredVertexArrayResource>(hri_.acvas);
}

ObjFileResource::~ObjFileResource()
{}

void ObjFileResource::instantiate_renderable(const InstantiationOptions& options) const
{
    hri_.instantiate_renderable(
        options,
        FixedArray<float, 3>{ 0.f, 0.f, 0.f },
        1.f);
}

std::shared_ptr<AnimatedColoredVertexArrays> ObjFileResource::get_animated_arrays() const {
    return hri_.get_animated_arrays(1.f);
}

void ObjFileResource::generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) {
    rva_->generate_triangle_rays(npoints, lengths, delete_triangles);
}

void ObjFileResource::generate_ray(const FixedArray<float, 3>& from, const FixedArray<float, 3>& to) {
    rva_->generate_ray(from, to);
}

std::shared_ptr<SceneNodeResource> ObjFileResource::generate_grind_lines(
    float edge_angle,
    float averaged_normal_angle,
    const ColoredVertexArrayFilter& filter) const
{
    return rva_->generate_grind_lines(edge_angle, averaged_normal_angle, filter);
}

std::shared_ptr<SceneNodeResource> ObjFileResource::generate_contour_edges() const {
    return rva_->generate_contour_edges();
}

void ObjFileResource::modify_physics_material_tags(
    PhysicsMaterial add,
    PhysicsMaterial remove,
    const ColoredVertexArrayFilter& filter)
{
    return rva_->modify_physics_material_tags(add, remove, filter);
}

void ObjFileResource::generate_instances() {
    hri_.generate_instances();
}

AggregateMode ObjFileResource::aggregate_mode() const {
    return rva_->aggregate_mode();
}

void ObjFileResource::print(std::ostream& ostr) const {
    rva_->print(ostr);
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
