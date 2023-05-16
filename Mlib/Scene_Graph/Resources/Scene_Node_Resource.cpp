#include "Scene_Node_Resource.hpp"
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

SceneNodeResource::SceneNodeResource()
{}

SceneNodeResource::~SceneNodeResource()
{}

void SceneNodeResource::preload() const {
    THROW_OR_ABORT("preload not implemented");
}

void SceneNodeResource::instantiate_renderable(const InstantiationOptions& options) const {
    THROW_OR_ABORT("instantiate_renderable not implemented");
}

TransformationMatrix<double, double, 3> SceneNodeResource::get_geographic_mapping(const TransformationMatrix<double, double, 3>& absolute_model_matrix) const {
    THROW_OR_ABORT("get_geographic_coordinates not implemented");
}

std::shared_ptr<AnimatedColoredVertexArrays> SceneNodeResource::get_animated_arrays() const {
    THROW_OR_ABORT("get_animated_arrays not implemented");
}

void SceneNodeResource::generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) {
    THROW_OR_ABORT("generate_triangle_rays not implemented");
}

void SceneNodeResource::generate_ray(const FixedArray<float, 3>& from, const FixedArray<float, 3>& to) {
    THROW_OR_ABORT("generate_ray not implemented");
}

std::shared_ptr<SceneNodeResource> SceneNodeResource::generate_grind_lines(
    float edge_angle,
    float averaged_normal_angle,
    const ColoredVertexArrayFilter& filter) const
{
    THROW_OR_ABORT("generate_grind_lines not implemented");
}

void SceneNodeResource::modify_physics_material_tags(
    PhysicsMaterial add,
    PhysicsMaterial remove,
    const ColoredVertexArrayFilter& filter)
{
    THROW_OR_ABORT("modify_physics_material_tags not implemented");
}

void SceneNodeResource::generate_instances() {
    THROW_OR_ABORT("generate_instances not implemented");
}

std::shared_ptr<SceneNodeResource> SceneNodeResource::generate_contour_edges() const
{
    THROW_OR_ABORT("generate_contour_edges not implemented");
}

AggregateMode SceneNodeResource::aggregate_mode() const {
    THROW_OR_ABORT("aggregate_mode not implemented");
}

std::list<SpawnPoint> SceneNodeResource::spawn_points() const {
    THROW_OR_ABORT("spawn_points not implemented");
}

std::map<WayPointLocation, PointsAndAdjacency<double, 3>> SceneNodeResource::way_points() const {
    THROW_OR_ABORT("way_points not implemented");
}

void SceneNodeResource::print(std::ostream& ostr) const {
    THROW_OR_ABORT("\"print\" not implemented");
}

void SceneNodeResource::set_relative_joint_poses(const std::map<std::string, OffsetAndQuaternion<float, float>>& poses) {
    THROW_OR_ABORT("set_relative_joint_poses not implemented");
}

void SceneNodeResource::downsample(size_t factor) {
    THROW_OR_ABORT("downsample not implemented");
}

void SceneNodeResource::import_bone_weights(
    const AnimatedColoredVertexArrays& other_acvas,
    float max_distance)
{
    THROW_OR_ABORT("import_bone_weights not implemented");
}

std::map<std::string, OffsetAndQuaternion<float, float>> SceneNodeResource::get_relative_poses(float seconds) const {
    THROW_OR_ABORT("get_relative_poses not implemented");
}

std::map<std::string, OffsetAndQuaternion<float, float>> SceneNodeResource::get_absolute_poses(float seconds) const {
    THROW_OR_ABORT("get_absolute_poses not implemented");
}

float SceneNodeResource::get_animation_duration() const {
    THROW_OR_ABORT("get_animation_duration not implemented");
}
