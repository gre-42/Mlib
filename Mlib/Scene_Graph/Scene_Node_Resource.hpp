#pragma once
#include <iosfwd>
#include <list>
#include <map>
#include <memory>
#include <string>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos>
class OffsetAndQuaternion;
template <class TData, size_t tndim>
struct PointsAndAdjacency;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;

struct AnimatedColoredVertexArrays;
template <class TPos>
struct ColoredVertexArray;
class Scene;
class SceneNode;
enum class WayPointLocation;
enum class AggregateMode;
struct SpawnPoint;
struct ColoredVertexArrayFilter;
struct RenderableResourceFilter;
enum class PhysicsMaterial;

class SceneNodeResource {
public:
    SceneNodeResource();
    virtual ~SceneNodeResource();
    // Misc
    virtual void instantiate_renderable(const std::string& name, SceneNode& scene_node, const RenderableResourceFilter& renderable_resource_filter) const;
    virtual TransformationMatrix<double, double, 3> get_geographic_mapping(const TransformationMatrix<double, double, 3>& absolute_model_matrix) const;
    virtual AggregateMode aggregate_mode() const;
    virtual std::list<SpawnPoint> spawn_points() const;
    virtual std::map<WayPointLocation, PointsAndAdjacency<double, 3>> way_points() const;
    virtual void print(std::ostream& ostr) const;

    // Animation
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_animated_arrays() const;
    virtual void import_bone_weights(
        const AnimatedColoredVertexArrays& other_acvas,
        float max_distance);
    virtual std::map<std::string, OffsetAndQuaternion<float, float>> get_poses(float seconds) const;
    virtual float get_animation_duration() const;

    // Modifiers
    virtual void set_relative_joint_poses(const std::map<std::string, OffsetAndQuaternion<float, float>>& poses);
    virtual void generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles = false);
    virtual void generate_ray(const FixedArray<float, 3>& from, const FixedArray<float, 3>& to);
    virtual void downsample(size_t factor);
    virtual void modify_physics_material_tags(
        PhysicsMaterial add,
        PhysicsMaterial remove,
        const ColoredVertexArrayFilter& filter);
    virtual void generate_instances();

    // Transformations
    virtual std::shared_ptr<SceneNodeResource> generate_grind_lines(
        float edge_angle,
        float averaged_normal_angle,
        const ColoredVertexArrayFilter& filter) const;
    virtual std::shared_ptr<SceneNodeResource> generate_contour_edges() const;
};

}
