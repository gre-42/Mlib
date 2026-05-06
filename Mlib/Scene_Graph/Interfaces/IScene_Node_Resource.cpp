#include "IScene_Node_Resource.hpp"
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Save_Obj.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <stdexcept>

using namespace Mlib;

ISceneNodeResource::ISceneNodeResource(std::string name)
    : name_{std::move(name)}
{}

ISceneNodeResource::~ISceneNodeResource() = default;

void ISceneNodeResource::preload(const RenderableResourceFilter& filter) const {
    throw std::runtime_error(name_ + ": preload not implemented");
}

void ISceneNodeResource::instantiate_child_renderable(const ChildInstantiationOptions& options) const {
    throw std::runtime_error(name_ + ": instantiate_child_renderable not implemented");
}

void ISceneNodeResource::instantiate_root_renderables(const RootInstantiationOptions& options) const {
    throw std::runtime_error(name_ + ": instantiate_root_renderables not implemented");
}

TransformationMatrix<double, double, 3> ISceneNodeResource::get_geographic_mapping(const TransformationMatrix<double, double, 3>& absolute_model_matrix) const {
    throw std::runtime_error(name_ + ": get_geographic_coordinates not implemented");
}

std::shared_ptr<AnimatedColoredVertexArrays> ISceneNodeResource::get_arrays(
    const ColoredVertexArrayFilter& filter) const
{
    throw std::runtime_error(name_ + ": get_arrays not implemented");
}

std::list<std::shared_ptr<AnimatedColoredVertexArrays>> ISceneNodeResource::get_rendering_arrays() const {
    throw std::runtime_error(name_ + ": get_rendering_arrays not implemented");
}

std::list<TypedMesh<std::shared_ptr<IIntersectable>>> ISceneNodeResource::get_intersectables() const {
    throw std::runtime_error(name_ + ": get_intersectables not implemented");
}

void ISceneNodeResource::generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) {
    throw std::runtime_error(name_ + ": generate_triangle_rays not implemented");
}

void ISceneNodeResource::generate_ray(const FixedArray<float, 3>& from, const FixedArray<float, 3>& to) {
    throw std::runtime_error(name_ + ": generate_ray not implemented");
}

std::shared_ptr<ISceneNodeResource> ISceneNodeResource::generate_grind_lines(
    float edge_angle,
    float averaged_normal_angle,
    const ColoredVertexArrayFilter& filter) const
{
    throw std::runtime_error(name_ + ": generate_grind_lines not implemented");
}

void ISceneNodeResource::modify_physics_material_tags(
    PhysicsMaterial add,
    PhysicsMaterial remove,
    const ColoredVertexArrayFilter& filter)
{
    throw std::runtime_error(name_ + ": modify_physics_material_tags not implemented");
}

void ISceneNodeResource::generate_instances() {
    throw std::runtime_error(name_ + ": generate_instances not implemented");
}

std::shared_ptr<ISceneNodeResource> ISceneNodeResource::generate_contour_edges() const
{
    throw std::runtime_error(name_ + ": generate_contour_edges not implemented");
}

void ISceneNodeResource::create_barrier_triangle_hitboxes(
    float depth,
    PhysicsMaterial destination_physics_material,
    const ColoredVertexArrayFilter& filter)
{
    throw std::runtime_error(name_ + ": create_barrier_triangle_hitboxes not implemented");
}

void ISceneNodeResource::smoothen_edges(
    SmoothnessTarget target,
    float smoothness,
    size_t niterations,
    float decay)
{
    throw std::runtime_error(name_ + ": smoothen_edges not implemented");
}

AggregateMode ISceneNodeResource::get_aggregate_mode() const {
    throw std::runtime_error(name_ + ": get_aggregate_mode not implemented");
}

PhysicsMaterial ISceneNodeResource::get_physics_material() const {
    throw std::runtime_error(name_ + ": get_physics_material not implemented");
}

std::list<SpawnPoint> ISceneNodeResource::get_spawn_points() const {
    throw std::runtime_error(name_ + ": get_spawn_points not implemented");
}

WayPointSandboxes ISceneNodeResource::get_way_points() const {
    throw std::runtime_error(name_ + ": get_way_points not implemented");
}

void ISceneNodeResource::save_to_obj_file(
    const std::string& prefix,
    const TransformationMatrix<float, ScenePos, 3>* model_matrix) const
{
    if (model_matrix != nullptr) {
        throw std::runtime_error("ISceneNodeResource::save_to_obj_file does not support model matrices");
    }
    auto acvas = get_rendering_arrays();

    for (const auto& [i, acva] : enumerate(acvas)) {
        if (!acva->scvas.empty()) {
            save_obj(prefix + "_s_" + std::to_string(i) + ".obj", acva->scvas);
        }
        if (!acva->dcvas.empty()) {
            save_obj(prefix + "_d_" + std::to_string(i) + ".obj", acva->dcvas);
        }
    }
}
    
void ISceneNodeResource::print(std::ostream& ostr) const {
    throw std::runtime_error(name_ + ": \"print\" not implemented");
}

void ISceneNodeResource::set_relative_joint_poses(const StringWithHashUnorderedMap<OffsetAndQuaternion<float, float>>& poses) {
    throw std::runtime_error(name_ + ": set_relative_joint_poses not implemented");
}

void ISceneNodeResource::downsample(size_t factor) {
    throw std::runtime_error(name_ + ": downsample not implemented");
}

void ISceneNodeResource::import_bone_weights(
    const AnimatedColoredVertexArrays& other_acvas,
    float max_distance)
{
    throw std::runtime_error(name_ + ": import_bone_weights not implemented");
}

StringWithHashUnorderedMap<OffsetAndQuaternion<float, float>> ISceneNodeResource::get_relative_poses(float seconds) const {
    throw std::runtime_error(name_ + ": get_relative_poses not implemented");
}

StringWithHashUnorderedMap<OffsetAndQuaternion<float, float>> ISceneNodeResource::get_absolute_poses(float seconds) const {
    throw std::runtime_error(name_ + ": get_absolute_poses not implemented");
}

float ISceneNodeResource::get_animation_duration() const {
    throw std::runtime_error(name_ + ": get_animation_duration not implemented");
}
