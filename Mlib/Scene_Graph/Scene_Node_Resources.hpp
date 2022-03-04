#pragma once
#include <functional>
#include <iosfwd>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace Mlib {

template <class TData, size_t n>
class TransformationMatrix;
class SceneNodeResource;
class SceneNode;
struct ColoredVertexArrayFilter;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TData>
class OffsetAndQuaternion;
struct AnimatedColoredVertexArrays;
template <class TData, size_t tndim>
struct PointsAndAdjacency;
enum class AggregateMode;
struct SpawnPoint;
enum class WayPointLocation;
struct ResourceFilter;
struct RenderableResourceFilter;
enum class PhysicsMaterial;

class SceneNodeResources {
public:
    SceneNodeResources();
    ~SceneNodeResources();

    // Misc
    void add_resource(
        const std::string& name,
        const std::shared_ptr<SceneNodeResource>& resource);
    void add_resource_loader(
        const std::string& name,
        const std::function<std::shared_ptr<SceneNodeResource>()>& resource);
    void instantiate_renderable(
        const std::string& resource_name,
        const std::string& instance_name,
        SceneNode& scene_node,
        const RenderableResourceFilter& renderable_resource_filter) const;
    void register_geographic_mapping(
        const std::string& resource_name,
        const std::string& instance_name,
        const TransformationMatrix<double, 3>& absolute_model_matrix);
    const TransformationMatrix<double, 3>* get_geographic_mapping(const std::string& name) const;
    std::shared_ptr<AnimatedColoredVertexArrays> get_animated_arrays(const std::string& name) const;
    AggregateMode aggregate_mode(const std::string& name) const;
    std::list<SpawnPoint> spawn_points(const std::string& name) const;
    std::map<WayPointLocation, PointsAndAdjacency<float, 3>> way_points(const std::string& name) const;
    void print(const std::string& name, std::ostream& ostr) const;

    // Animation
    void set_relative_joint_poses(const std::string& name, const std::map<std::string, OffsetAndQuaternion<float>>& poses);
    std::map<std::string, OffsetAndQuaternion<float>> get_poses(const std::string& name, float seconds) const;
    float get_animation_duration(const std::string& name) const;

    // Modifiers
    void generate_triangle_rays(const std::string& name, size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles = false);
    void generate_ray(const std::string& name, const FixedArray<float, 3>& from, const FixedArray<float, 3>& to);
    void downsample(const std::string& name, size_t factor);
    void import_bone_weights(
        const std::string& destination,
        const std::string& source,
        float max_distance);
    void add_companion(
        const std::string& resource_name,
        const std::string& companion_resource_name,
        const RenderableResourceFilter& renderable_resource_filter);
    void modify_physics_material_tags(
        const std::string& name,
        const ColoredVertexArrayFilter& filter,
        PhysicsMaterial add,
        PhysicsMaterial remove);
    void generate_instances(const std::string& name);

    // Transformations
    void generate_grind_lines(
        const std::string& source_name,
        const std::string& dest_name,
        float edge_angle,
        float averaged_normal_angle,
        const ColoredVertexArrayFilter& filter);
    void generate_contour_edges(
        const std::string& source_name,
        const std::string& dest_name);
private:
    std::shared_ptr<SceneNodeResource> get_resource(const std::string& name) const;
    void add_modifier(
        const std::string& resource_name,
        const std::function<void(SceneNodeResource&)>& modifier);
    mutable std::map<std::string, std::shared_ptr<SceneNodeResource>> resources_;
    std::map<std::string, TransformationMatrix<double, 3>> geographic_mappings_;
    std::map<std::string, std::list<std::pair<std::string, RenderableResourceFilter>>> companions_;
    std::map<std::string, std::function<std::shared_ptr<SceneNodeResource>()>> resource_loaders_;
    mutable std::map<std::string, std::list<std::function<void(SceneNodeResource&)>>> modifiers_;
    mutable std::recursive_mutex mutex_;
};

}
