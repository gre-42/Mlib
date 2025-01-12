#pragma once
#include <Mlib/Scene_Graph/Interfaces/Way_Points_Fwd.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <cstdint>
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
template <class TPoint>
struct PointsAndAdjacency;
enum class WayPointLocation;
enum class JoinedWayPointSandbox;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
template <class T>
struct TypedMesh;

struct AnimatedColoredVertexArrays;
template <class TPos>
class ColoredVertexArray;
class IIntersectable;
class Scene;
class SceneNode;
enum class AggregateMode;
struct SpawnPoint;
struct ColoredVertexArrayFilter;
struct RenderableResourceFilter;
enum class PhysicsMaterial: uint32_t;
class ISupplyDepots;
struct ChildInstantiationOptions;
struct RootInstantiationOptions;
enum class SmoothnessTarget;

class ISceneNodeResource {
public:
    ISceneNodeResource();
    virtual ~ISceneNodeResource();
    // Misc
    virtual void preload(const RenderableResourceFilter& filter) const;
    virtual void instantiate_child_renderable(const ChildInstantiationOptions& options) const;
    virtual void instantiate_root_renderables(const RootInstantiationOptions& options) const;
    virtual TransformationMatrix<double, double, 3> get_geographic_mapping(const TransformationMatrix<double, double, 3>& absolute_model_matrix) const;
    virtual AggregateMode get_aggregate_mode() const;
    virtual std::list<SpawnPoint> get_spawn_points() const;
    virtual WayPointSandboxes get_way_points() const;

    // Output
    virtual void save_to_obj_file(
        const std::string& prefix,
        const TransformationMatrix<SceneDir, ScenePos, 3>* model_matrix) const;
    virtual void print(std::ostream& ostr) const;

    // Animation
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_physics_arrays() const;
    virtual std::list<std::shared_ptr<AnimatedColoredVertexArrays>> get_rendering_arrays() const;
    virtual std::list<TypedMesh<std::shared_ptr<IIntersectable>>> get_intersectables() const;
    virtual void import_bone_weights(
        const AnimatedColoredVertexArrays& other_acvas,
        float max_distance);
    virtual std::map<std::string, OffsetAndQuaternion<float, float>> get_relative_poses(float seconds) const;
    virtual std::map<std::string, OffsetAndQuaternion<float, float>> get_absolute_poses(float seconds) const;
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
    virtual void create_barrier_triangle_hitboxes(
        float depth,
        PhysicsMaterial destination_physics_material,
        const ColoredVertexArrayFilter& filter);
    virtual void smoothen_edges(
        SmoothnessTarget target,
        float smoothness,
        size_t niterations,
        float decay = 0.97f);

    // Transformations
    virtual std::shared_ptr<ISceneNodeResource> generate_grind_lines(
        float edge_angle,
        float averaged_normal_angle,
        const ColoredVertexArrayFilter& filter) const;
    virtual std::shared_ptr<ISceneNodeResource> generate_contour_edges() const;
};

}
