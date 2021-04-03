#include "Scene_Node_Resource.hpp"
#include <Mlib/Math/Transformation_Matrix.hpp>

using namespace Mlib;

void SceneNodeResource::instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter) const {
    throw std::runtime_error("instantiate_renderable not implemented");
}
TransformationMatrix<double, 3> SceneNodeResource::get_geographic_mapping(const SceneNode& scene_node) const {
    throw std::runtime_error("get_geographic_coordinates not implemented");
}
std::shared_ptr<AnimatedColoredVertexArrays> SceneNodeResource::get_animated_arrays() const {
    throw std::runtime_error("get_animated_arrays not implemented");
}
void SceneNodeResource::generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) {
    throw std::runtime_error("generate_triangle_rays not implemented");
}
void SceneNodeResource::generate_ray(const FixedArray<float, 3>& from, const FixedArray<float, 3>& to) {
    throw std::runtime_error("generate_ray not implemented");
}
AggregateMode SceneNodeResource::aggregate_mode() const {
    throw std::runtime_error("aggregate_mode not implemented");
}
std::list<SpawnPoint> SceneNodeResource::spawn_points() const {
    throw std::runtime_error("spawn_points not implemented");
}
std::map<WayPointLocation, PointsAndAdjacency<float, 2>> SceneNodeResource::way_points() const {
    throw std::runtime_error("way_points not implemented");
}
void SceneNodeResource::set_relative_joint_poses(const std::map<std::string, OffsetAndQuaternion<float>>& poses) {
    throw std::runtime_error("set_relative_joint_poses not implemented");
}
void SceneNodeResource::downsample(size_t factor) {
    throw std::runtime_error("downsample not implemented");
}
void SceneNodeResource::import_bone_weights(
    const AnimatedColoredVertexArrays& other_acvas,
    float max_distance)
{
    throw std::runtime_error("import_bone_weights not implemented");
}
