#pragma once
#include <Mlib/Map/String_With_Hash_Unordered_Map.hpp>
#include <Mlib/Map/String_With_Hash_Unordered_Map.hpp>
#include <Mlib/Map/Threadsafe_String_With_Hash_Unordered_Map.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <Mlib/Scene_Graph/Interfaces/Way_Points_Fwd.hpp>
#include <Mlib/Scene_Graph/Preload_Behavior.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos>
class OffsetAndQuaternion;
template <class TData, size_t n>
struct FixedScaledUnitVector;

class ISceneNodeResource;
class SceneNode;

template <class TPos>
class ColoredVertexArray;
struct ColoredVertexArrayFilter;
struct AnimatedColoredVertexArrays;
template <class T>
struct TypedMesh;
class IIntersectable;
struct SpawnPoint;
struct RenderableResourceFilter;
struct ChildInstantiationOptions;
struct RootInstantiationOptions;
template <class TPosition>
struct InstanceInformation;

enum class AggregateMode;
enum class PhysicsMaterial: uint32_t;
enum class SmoothnessTarget;

class SceneNodeResources {
public:
    SceneNodeResources();
    ~SceneNodeResources();

    // Preload
    void write_loaded_resources(const std::string& filename) const;
    void preload_many(
        const std::string& filename,
        const RenderableResourceFilter& filter) const;
    void preload_single(
        const VariableAndHash<std::string>& name,
        const RenderableResourceFilter& filter) const;

    // Misc
    void add_resource(
        const VariableAndHash<std::string>& name,
        const std::shared_ptr<ISceneNodeResource>& resource);
    void add_resource_loader(
        const VariableAndHash<std::string>& name,
        const std::function<std::shared_ptr<ISceneNodeResource>()>& resource);
    void instantiate_child_renderable(
        const VariableAndHash<std::string>& resource_name,
        const ChildInstantiationOptions& options,
        PreloadBehavior preload_behavior = PreloadBehavior::PRELOAD,
        unsigned int recursion_depth = 0) const;
    void instantiate_root_renderables(
        const VariableAndHash<std::string>& resource_name,
        const RootInstantiationOptions& options,
        PreloadBehavior preload_behavior = PreloadBehavior::PRELOAD,
        unsigned int recursion_depth = 0) const;
    // Instantiables
    void add_instantiable(
        const VariableAndHash<std::string>& name,
        const InstanceInformation<ScenePos>& instantiable);
    const InstanceInformation<ScenePos>& instantiable(
        const VariableAndHash<std::string>& name) const;
    // Geographic mapping
    void register_geographic_mapping(
        const VariableAndHash<std::string>& resource_name,
        const VariableAndHash<std::string>& instance_name,
        const TransformationMatrix<double, double, 3>& absolute_model_matrix);
    TransformationMatrix<double, double, 3> get_geographic_mapping(
        const VariableAndHash<std::string>& name,
        const TransformationMatrix<double, double, 3>& absolute_model_matrix) const;
    const TransformationMatrix<double, double, 3>* get_geographic_mapping(const VariableAndHash<std::string>& name) const;
    // Wind
    void register_wind(
        VariableAndHash<std::string> name,
        const FixedArray<float, 3>& wind);
    const FixedScaledUnitVector<float, 3>* get_wind(const VariableAndHash<std::string>& name) const;
    // Gravity
    void register_gravity(
        VariableAndHash<std::string> name,
        const FixedArray<float, 3>& gravity);
    const FixedScaledUnitVector<float, 3>* get_gravity(const VariableAndHash<std::string>& name) const;
    // Navigation
    std::list<SpawnPoint> get_spawn_points(const VariableAndHash<std::string>& name) const;
    WayPointSandboxes get_way_points(const VariableAndHash<std::string>& name) const;
    // Other
    AggregateMode aggregate_mode(const VariableAndHash<std::string>& name) const;
    PhysicsMaterial physics_material(const VariableAndHash<std::string>& name) const;
    void add_companion(
        const VariableAndHash<std::string>& resource_name,
        const VariableAndHash<std::string>& companion_resource_name,
        const RenderableResourceFilter& renderable_resource_filter);

    // Output
    void save_to_obj_file(
        const VariableAndHash<std::string>& resource_name,
        const std::string& prefix,
        const TransformationMatrix<float, ScenePos, 3>* model_matrix) const;
    void print(const VariableAndHash<std::string>& name, std::ostream& ostr) const;

    // Animation
    std::shared_ptr<AnimatedColoredVertexArrays> get_arrays(
        const VariableAndHash<std::string>& name,
        const ColoredVertexArrayFilter& filter) const;
    std::list<std::shared_ptr<AnimatedColoredVertexArrays>> get_rendering_arrays(const VariableAndHash<std::string>& name) const;
    std::shared_ptr<ColoredVertexArray<float>> get_single_precision_array(
        const VariableAndHash<std::string>& name,
        const ColoredVertexArrayFilter& filter) const;
    std::list<std::shared_ptr<ColoredVertexArray<float>>> get_single_precision_arrays(
        const VariableAndHash<std::string>& name,
        const ColoredVertexArrayFilter& filter) const;
    std::list<TypedMesh<std::shared_ptr<IIntersectable>>> get_intersectables(const VariableAndHash<std::string>& name) const;
    void set_relative_joint_poses(const VariableAndHash<std::string>& name, const StringWithHashUnorderedMap<OffsetAndQuaternion<float, float>>& poses);
    StringWithHashUnorderedMap<OffsetAndQuaternion<float, float>> get_relative_poses(const VariableAndHash<std::string>& name, float seconds) const;
    StringWithHashUnorderedMap<OffsetAndQuaternion<float, float>> get_absolute_poses(const VariableAndHash<std::string>& name, float seconds) const;
    float get_animation_duration(const VariableAndHash<std::string>& name) const;

    // Modifiers
    void add_modifier(
        const VariableAndHash<std::string>& resource_name,
        const std::function<void(ISceneNodeResource&)>& modifier);
    void generate_triangle_rays(const VariableAndHash<std::string>& name, size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles = false);
    void generate_ray(const VariableAndHash<std::string>& name, const FixedArray<float, 3>& from, const FixedArray<float, 3>& to);
    void downsample(const VariableAndHash<std::string>& name, size_t factor);
    void import_bone_weights(
        const VariableAndHash<std::string>& destination,
        const VariableAndHash<std::string>& source,
        float max_distance,
        const ColoredVertexArrayFilter& filter);
    void modify_physics_material_tags(
        const VariableAndHash<std::string>& name,
        const ColoredVertexArrayFilter& filter,
        PhysicsMaterial add,
        PhysicsMaterial remove);
    void generate_instances(const VariableAndHash<std::string>& name);
    void create_barrier_triangle_hitboxes(
        const VariableAndHash<std::string>& resource_name,
        float depth,
        PhysicsMaterial destination_physics_material,
        const ColoredVertexArrayFilter& filter);
    void smoothen_edges(
        const VariableAndHash<std::string>& resource_name,
        SmoothnessTarget target,
        float smoothness,
        size_t niterations,
        float decay = 0.97f);

    // Transformations
    void generate_grind_lines(
        const VariableAndHash<std::string>& source_name,
        const VariableAndHash<std::string>& dest_name,
        float edge_angle,
        float averaged_normal_angle,
        const ColoredVertexArrayFilter& filter);
    void generate_contour_edges(
        const VariableAndHash<std::string>& source_name,
        const VariableAndHash<std::string>& dest_name);
private:
    std::shared_ptr<ISceneNodeResource> get_resource(const VariableAndHash<std::string>& name) const;
    mutable ThreadsafeStringWithHashUnorderedMap<std::shared_ptr<ISceneNodeResource>> resources_;
    ThreadsafeStringWithHashUnorderedMap<InstanceInformation<ScenePos>> instantiables_;
    ThreadsafeStringWithHashUnorderedMap<TransformationMatrix<double, double, 3>> geographic_mappings_;
    ThreadsafeStringWithHashUnorderedMap<FixedScaledUnitVector<float, 3>> wind_;
    ThreadsafeStringWithHashUnorderedMap<FixedScaledUnitVector<float, 3>> gravity_;
    StringWithHashUnorderedMap<std::list<std::pair<VariableAndHash<std::string>, RenderableResourceFilter>>> companions_;
    StringWithHashUnorderedMap<std::function<std::shared_ptr<ISceneNodeResource>()>> resource_loaders_;
    mutable StringWithHashUnorderedMap<std::list<std::function<void(ISceneNodeResource&)>>> modifiers_;
    mutable SafeAtomicRecursiveSharedMutex mutex_;
    mutable SafeAtomicRecursiveSharedMutex companion_mutex_;
};

}
