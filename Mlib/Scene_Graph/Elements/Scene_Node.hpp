#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Math/Transformation/Quaternion_Series.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Observers.hpp>
#include <Mlib/Memory/Memory.hpp>
#include <Mlib/Memory/Shared_Ptrs.hpp>
#include <Mlib/Object.hpp>
#include <Mlib/Scene_Graph/Elements/Color_Style.hpp>
#include <Mlib/Threads/Safe_Recursive_Shared_Mutex.hpp>
#include <atomic>
#include <cstdint>
#include <iosfwd>
#include <map>
#include <memory>
#include <optional>
#include <set>

namespace Mlib {

template <class TData, size_t tndim>
class AxisAlignedBoundingBox;

struct SceneGraphConfig;
struct RenderConfig;
class Scene;
class SceneNode;
struct AnimationState;
class AnimationStateUpdater;
class SceneNodeResources;
class Renderable;
class NodeHider;
class AbsoluteMovable;
class Camera;
class NodeModifier;
class RelativeMovable;
class AbsoluteObserver;
struct Light;
enum class ExternalRenderPassType;
struct ExternalRenderPass;
template <class TPos>
class ColoredVertexArray;
class SmallInstancesQueues;
class LargeInstancesQueue;

struct Blended {
    int z_order;
    FixedArray<double, 4, 4> mvp;
    TransformationMatrix<float, double, 3> m;
    const Renderable* renderable;
    const AnimationState* animation_state;
    ColorStyle color_style;
    inline std::pair<int, double> sorting_key() const {
        return { z_order, mvp(2u, 3u) };
    }
};

struct PositionAndYAngle {
    FixedArray<double, 3> position;
    float yangle;
    uint32_t billboard_id;
};

struct SceneNodeInstances {
    bool is_registered;
    DanglingUniquePtr<SceneNode> scene_node;
    double max_center_distance;
    Bvh<double, PositionAndYAngle, 3> small_instances;
    std::list<PositionAndYAngle> large_instances;
};

struct SceneNodeChild {
    bool is_registered;
    DanglingUniquePtr<SceneNode> scene_node;
};

struct SceneNodeBone {
    std::string name;
    float smoothness;
    float rotation_strength;
};

enum class SceneNodeState {
    DETACHED,
    STATIC,
    DYNAMIC
};

enum class SceneNodeVisibility {
    VISIBLE,
    INVISIBLE
};

enum class ChildRegistrationState {
    NOT_REGISTERED,
    REGISTERED
};

enum class ChildParentState {
    PARENT_NOT_SET,
    PARENT_ALREADY_SET
};

class SceneNode: public Object {
    template <class TAbsoluteMovable>
    friend class AbsoluteMovableSetter;
    SceneNode(const SceneNode& other) = delete;
    SceneNode& operator = (const SceneNode& other) = delete;
public:
    explicit SceneNode();
    SceneNode(
        const FixedArray<double, 3>& position,
        const FixedArray<float, 3>& rotation,
        float scale);
    ~SceneNode();
    bool shutting_down() const;
    AbsoluteMovable& get_absolute_movable() const;
    RelativeMovable& get_relative_movable() const;
    NodeModifier& get_node_modifier() const;
    AbsoluteObserver& get_absolute_observer() const;
    bool has_node_modifier() const;
    void set_relative_movable(const observer_ptr<RelativeMovable, DanglingRef<const SceneNode>>& relative_movable);
    void set_node_modifier(std::unique_ptr<NodeModifier>&& node_modifier);
    void insert_node_hider(NodeHider& node_hider);
    void remove_node_hider(NodeHider& node_hider);
    void set_absolute_observer(const observer_ptr<AbsoluteObserver, DanglingRef<const SceneNode>>& absolute_observer);
    void add_renderable(
        const std::string& name,
        const std::shared_ptr<const Renderable>& renderable);
    void add_child(
        const std::string& name,
        DanglingUniquePtr<SceneNode>&& node,
        ChildRegistrationState child_registration_state = ChildRegistrationState::NOT_REGISTERED,
        ChildParentState =  ChildParentState::PARENT_NOT_SET);
    void set_parent(DanglingRef<SceneNode> parent);
    bool has_parent() const;
    DanglingRef<SceneNode> parent();
    DanglingRef<const SceneNode> parent() const;
    void clear_renderable_instance(const std::string& name);
    void clear_absolute_observer();
    void clear();
    DanglingRef<SceneNode> get_child(const std::string& name);
    DanglingRef<const SceneNode> get_child(const std::string& name) const;
    void remove_child(const std::string& name);
    bool contains_child(const std::string& name) const;
    void add_aggregate_child(
        const std::string& name,
        DanglingUniquePtr<SceneNode>&& node,
        ChildRegistrationState child_registration_state = ChildRegistrationState::NOT_REGISTERED,
        ChildParentState child_parent_state =  ChildParentState::PARENT_NOT_SET);
    void add_instances_child(
        const std::string& name,
        DanglingUniquePtr<SceneNode>&& node,
        ChildRegistrationState child_registration_state = ChildRegistrationState::NOT_REGISTERED,
        ChildParentState child_parent_state =  ChildParentState::PARENT_NOT_SET);
    void add_instances_position(
        const std::string& name,
        const FixedArray<double, 3>& position,
        float yangle,
        uint32_t billboard_id);
    void optimize_instances_search_time(std::ostream& ostr) const;
    bool has_camera() const;
    void set_camera(std::unique_ptr<Camera>&& camera);
    Camera& get_camera() const;
    void add_light(std::unique_ptr<Light>&& light);
    void visit(
        const TransformationMatrix<float, double, 3>& parent_m,
        const std::function<void(
            const TransformationMatrix<float, double, 3>& m,
            const std::map<std::string, std::shared_ptr<const Renderable>>& renderables)>& func) const;
    void move(
        const TransformationMatrix<float, double, 3>& v,
        float dt,
        std::chrono::steady_clock::time_point time,
        SceneNodeResources* scene_node_resources,
        const AnimationState* animation_state);
    bool requires_render_pass(ExternalRenderPassType render_pass) const;
    void render(
        const FixedArray<double, 4, 4>& parent_mvp,
        const TransformationMatrix<float, double, 3>& parent_m,
        const TransformationMatrix<float, double, 3>& iv,
        DanglingRef<const SceneNode> camera_node,
        const std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
        std::list<Blended>& blended,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        const ExternalRenderPass& external_render_pass,
        const AnimationState* animation_state,
        const std::list<const ColorStyle*>& color_styles,
        SceneNodeVisibility visibility = SceneNodeVisibility::VISIBLE) const;
    void append_sorted_aggregates_to_queue(
        const FixedArray<double, 4, 4>& parent_mvp,
        const TransformationMatrix<float, double, 3>& parent_m,
        const FixedArray<double, 3>& offset,
        std::list<std::pair<float, std::shared_ptr<ColoredVertexArray<float>>>>& aggregate_queue,
        const SceneGraphConfig& scene_graph_config,
        const ExternalRenderPass& external_render_pass) const;
    void append_large_aggregates_to_queue(
        const TransformationMatrix<float, double, 3>& parent_m,
        const FixedArray<double, 3>& offset,
        std::list<std::shared_ptr<ColoredVertexArray<float>>>& aggregate_queue,
        const SceneGraphConfig& scene_graph_config) const;
    void append_small_instances_to_queue(
        const FixedArray<double, 4, 4>& parent_mvp,
        const TransformationMatrix<float, double, 3>& parent_m,
        const TransformationMatrix<float, double, 3>& iv,
        const FixedArray<double, 3>& offset,
        const PositionAndYAngle& delta_pose,
        SmallInstancesQueues& instances_queues,
        const SceneGraphConfig& scene_graph_config) const;
    void append_large_instances_to_queue(
        const FixedArray<double, 4, 4>& parent_mvp,
        const TransformationMatrix<float, double, 3>& parent_m,
        const FixedArray<double, 3>& offset,
        const PositionAndYAngle& delta_pose,
        LargeInstancesQueue& instances_queue,
        const SceneGraphConfig& scene_graph_config) const;
    void append_lights_to_queue(
        const TransformationMatrix<float, double, 3>& parent_m,
        std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights) const;
    const FixedArray<double, 3>& position() const;
    FixedArray<float, 3> rotation() const;
    float scale() const;
    void set_position(const FixedArray<double, 3>& position);
    void set_rotation(const FixedArray<float, 3>& rotation);
    void set_scale(float scale);
    void set_relative_pose(
        const FixedArray<double, 3>& position,
        const FixedArray<float, 3>& rotation,
        float scale);
    void set_absolute_pose(
        const FixedArray<double, 3>& position,
        const FixedArray<float, 3>& rotation,
        float scale);
    TransformationMatrix<float, double, 3> relative_model_matrix() const;
    TransformationMatrix<float, double, 3> absolute_model_matrix(std::chrono::steady_clock::time_point time = std::chrono::steady_clock::time_point()) const;
    TransformationMatrix<float, double, 3> relative_view_matrix() const;
    TransformationMatrix<float, double, 3> absolute_view_matrix(std::chrono::steady_clock::time_point time = std::chrono::steady_clock::time_point()) const;
    std::optional<AxisAlignedBoundingBox<float, 3>> relative_aabb() const;
    double max_center_distance(uint32_t billboard_id) const;
    void print(std::ostream& ostr, size_t recursion_depth = 0) const;
    bool has_color_style(const std::string& name) const;
    ColorStyle& color_style(const std::string& name);
    const ColorStyle& color_style(const std::string& name) const;
    void add_color_style(std::unique_ptr<ColorStyle>&& color_style);
    void set_animation_state(std::unique_ptr<AnimationState>&& animation_state);
    void set_animation_state_updater(std::unique_ptr<AnimationStateUpdater>&& animation_state_updater);
    bool to_be_deleted() const;
    void set_bone(const SceneNodeBone& bone);
    void set_periodic_animation(const std::string& name);
    void set_aperiodic_animation(const std::string& name);
    void set_scene_and_state(Scene& scene, SceneNodeState state);
    Scene& scene();
    const Scene& scene() const;
    void set_debug_message(std::string message);
    std::string debug_message() const;
    mutable DestructionObservers<DanglingRef<const SceneNode>> clearing_observers;
    mutable DestructionObservers<DanglingRef<const SceneNode>> destruction_observers;
    mutable SharedPtrs clearing_pointers;
    mutable SharedPtrs destruction_pointers;
private:
    void set_scene_and_state_unsafe(Scene& scene, SceneNodeState state);
    void setup_child_unsafe(
        const std::string& name,
        DanglingRef<SceneNode> node,
        ChildRegistrationState child_registration_state,
        ChildParentState child_parent_state);
    void clear_unsafe();
    TransformationMatrix<float, double, 3> relative_model_matrix_unsafe(std::chrono::steady_clock::time_point time = std::chrono::steady_clock::time_point()) const;
    TransformationMatrix<float, double, 3> relative_view_matrix_unsafe(std::chrono::steady_clock::time_point time = std::chrono::steady_clock::time_point()) const;
    Scene* scene_;
    DanglingPtr<SceneNode> parent_;
    AbsoluteMovable* absolute_movable_;
    RelativeMovable* relative_movable_;
    std::unique_ptr<NodeModifier> node_modifier_;
    std::set<NodeHider*> node_hiders_;
    AbsoluteObserver* absolute_observer_;
    DestructionObserver<DanglingRef<const SceneNode>>* absolute_destruction_observer_;
    std::unique_ptr<Camera> camera_;
    std::map<std::string, std::shared_ptr<const Renderable>> renderables_;
    std::map<std::string, SceneNodeChild> children_;
    std::map<std::string, SceneNodeChild> aggregate_children_;
    std::map<std::string, SceneNodeInstances> instances_children_;
    std::list<std::unique_ptr<Light>> lights_;
    OffsetAndQuaternion<float, double> trafo_;
    QuaternionSeries<float, double, 3> trafo_history_;
    float scale_;
    FixedArray<float, 3, 3> rotation_matrix_;
    std::unique_ptr<AnimationState> animation_state_;
    std::list<std::unique_ptr<ColorStyle>> color_styles_;
    std::unique_ptr<AnimationStateUpdater> animation_state_updater_;
    SceneNodeBone bone_;
    std::string periodic_animation_;
    std::string aperiodic_animation_;
    SceneNodeState state_;
    mutable SafeRecursiveSharedMutex mutex_;
    std::atomic_bool shutting_down_;
    std::string debug_message_;
};

std::ostream& operator << (std::ostream& ostr, DanglingPtr<const SceneNode> node);

}
