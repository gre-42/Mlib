#pragma once
#include <Mlib/Billboard_Id.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Math/Transformation/Quaternion_Series.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Memory/Destruction_Observers.hpp>
#include <Mlib/Memory/Memory.hpp>
#include <Mlib/Memory/Shared_Ptrs.hpp>
#include <Mlib/Object.hpp>
#include <Mlib/Scene_Graph/Elements/Color_Style.hpp>
#include <Mlib/Scene_Graph/Interpolation.hpp>
#include <Mlib/Scene_Graph/Pose_Interpolation_Mode.hpp>
#include <Mlib/Scene_Precision.hpp>
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
template <class TPos, size_t tndim>
class BoundingSphere;

struct ColoredVertexArrayFilter;
struct SceneGraphConfig;
struct RenderConfig;
class Scene;
class SceneNode;
struct AnimationState;
class AnimationStateUpdater;
class SceneNodeResources;
class Renderable;
class INodeHider;
class IAbsoluteMovable;
class Camera;
class INodeModifier;
class IRelativeMovable;
class IAbsoluteObserver;
class IDynamicLights;
struct Light;
struct Skidmark;
enum class ExternalRenderPassType;
struct ExternalRenderPass;
template <class TPos>
class ColoredVertexArray;
class SmallInstancesQueues;
class LargeInstancesQueue;
class Blended;
template <class T>
struct Bijection;

template <class TPosition>
struct PositionAndYAngleAndBillboardId {
    FixedArray<TPosition, 3> position;
    SceneDir yangle;
    BillboardId billboard_id;
    inline AxisAlignedBoundingBox<TPosition, 3> aabb() const {
        return AxisAlignedBoundingBox<TPosition, 3>::from_point(position);
    }
    inline PositionAndYAngleAndBillboardId& payload() {
        return *this;
    }
    inline const PositionAndYAngleAndBillboardId& payload() const {
        return *this;
    }
};

inline PositionAndYAngleAndBillboardId<CompressedScenePos> operator + (
    const PositionAndYAngleAndBillboardId<HalfCompressedScenePos>& a,
    const FixedArray<CompressedScenePos, 3>& reference)
{
    return { a.position.casted<CompressedScenePos>() + reference, a.yangle, a.billboard_id };
}

inline PositionAndYAngleAndBillboardId<HalfCompressedScenePos> operator - (
    const PositionAndYAngleAndBillboardId<CompressedScenePos>& a,
    const FixedArray<CompressedScenePos, 3>& reference)
{
    auto p = a.position - reference;
    auto cp = p.casted<HalfCompressedScenePos>();
    if (any(cp.casted<CompressedScenePos>() != p)) {
        THROW_OR_ABORT("PositionAndYAngleAndBillboardId: Could not compress scene position");
    }
    return { cp, a.yangle, a.billboard_id };
}

template <class TPosition>
struct PositionAndBillboardId {
    FixedArray<TPosition, 3> position;
    uint16_t billboard_id;
    inline AxisAlignedBoundingBox<TPosition, 3> aabb() const
    {
        return AxisAlignedBoundingBox<TPosition, 3>::from_point(position);
    }
    inline PositionAndBillboardId& payload() {
        return *this;
    }
    inline const PositionAndBillboardId& payload() const {
        return *this;
    }
};
static_assert(sizeof(PositionAndBillboardId<HalfCompressedScenePos>) == 8);

inline PositionAndBillboardId<CompressedScenePos> operator + (
    const PositionAndBillboardId<HalfCompressedScenePos>& a,
    const FixedArray<CompressedScenePos, 3>& reference)
{
    return { a.position.casted<CompressedScenePos>() + reference, a.billboard_id };
}

inline PositionAndBillboardId<HalfCompressedScenePos> operator - (
    const PositionAndBillboardId<CompressedScenePos>& a,
    const FixedArray<CompressedScenePos, 3>& reference)
{
    auto p = a.position - reference;
    auto cp = p.casted<HalfCompressedScenePos>();
    if (any(cp.casted<CompressedScenePos>() != p)) {
        THROW_OR_ABORT("PositionAndBillboardId: Could not compress scene position");
    }
    return { cp, a.billboard_id };
}

class BillboardContainer {
public:
    auto& add(const PositionAndYAngleAndBillboardId<CompressedScenePos>& pyb) {
        if (empty()) {
            reference_point_ = pyb.aabb().min;
        }
        return pybs_.emplace_back(pyb - reference_point_);
    }
    auto& add(const PositionAndBillboardId<CompressedScenePos>& pb) {
        if (empty()) {
            reference_point_ = pb.aabb().min;
        }
        return pbs_.emplace_back(pb - reference_point_);
    }
    void fill(auto& container) const {
        for (const auto& d : pybs_) {
            container.insert(d + reference_point_);
        }
        for (const auto& d : pbs_) {
            container.insert(d + reference_point_);
        }
    }
    bool visit(const auto& aabb, const auto& pyb_visitor, const auto& pb_visitor) const {
        for (const auto& d : pybs_) {
            auto ud = d + reference_point_;
            if (aabb.intersects(ud.aabb())) {
                if (!pyb_visitor(ud.payload())) {
                    return false;
                }
            }
        }
        for (const auto& d : pbs_) {
            auto ud = d + reference_point_;
            if (aabb.intersects(ud.aabb())) {
                if (!pb_visitor(ud.payload())) {
                    return false;
                }
            }
        }
        return true;
    }
    bool visit_all(const auto& pyb_visitor, const auto& pb_visitor) const {
        for (const auto& d : pybs_) {
            if (!pyb_visitor(d + reference_point_)) {
                return false;
            }
        }
        for (const auto& d : pbs_) {
            if (!pb_visitor(d + reference_point_)) {
                return false;
            }
        }
        return true;
    }
    bool visit_pairs(const auto& aabb, const auto& pyb_visitor, const auto& pb_visitor) const {
        for (const auto& d : pybs_) {
            auto ud = d + reference_point_;
            if (aabb.intersects(ud.aabb())) {
                if (!pyb_visitor(ud)) {
                    return false;
                }
            }
        }
        for (const auto& d : pbs_) {
            auto ud = d + reference_point_;
            if (aabb.intersects(ud.aabb())) {
                if (!pb_visitor(ud)) {
                    return false;
                }
            }
        }
        return true;
    }
    void print(std::ostream& ostr, size_t rec = 0) const {
        for (const auto& d : pybs_) {
            (d + reference_point_).aabb().print(ostr, rec + 1);
        }
        for (const auto& d : pbs_) {
            (d + reference_point_).aabb().print(ostr, rec + 1);
        }
    }
    bool empty() const {
        return pybs_.empty() && pbs_.empty();
    }
    size_t size() const {
        return pybs_.size() + pbs_.size();
    }
    void clear() {
        pybs_.clear();
        pbs_.clear();
    }
private:
    FixedArray<CompressedScenePos, 3> reference_point_ = uninitialized;
    std::vector<PositionAndYAngleAndBillboardId<HalfCompressedScenePos>> pybs_;
    std::vector<PositionAndBillboardId<HalfCompressedScenePos>> pbs_;
};

struct SceneNodeInstances {
    using SmallInstances = GenericBvh<CompressedScenePos, 3, BillboardContainer>;

    bool is_registered;
    DanglingUniquePtr<SceneNode> scene_node;
    CompressedScenePos max_center_distance;
    SmallInstances small_instances;
    std::list<PositionAndYAngleAndBillboardId<CompressedScenePos>> large_instances;
    mutable SafeAtomicSharedMutex mutex;
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

enum class LockingStrategy {
    NO_LOCK,
    ACQUIRE_LOCK
};

enum class RenderingStrategies;

class RenderableWithStyle;

static const auto INITIAL_POSE = std::chrono::steady_clock::time_point();
static const auto SUCCESSOR_POSE = std::nullopt;

class SceneNode: public virtual Object {
    template <class TAbsoluteMovable>
    friend class AbsoluteMovableSetter;
    SceneNode(const SceneNode& other) = delete;
    SceneNode& operator = (const SceneNode& other) = delete;
public:
    explicit SceneNode(
        PoseInterpolationMode interpolation_mode = PoseInterpolationMode::ENABLED);
    SceneNode(
        const FixedArray<ScenePos, 3>& position,
        const FixedArray<float, 3>& rotation,
        float scale,
        PoseInterpolationMode interpolation_mode = PoseInterpolationMode::ENABLED);
    virtual ~SceneNode() override;
    virtual void shutdown();
    bool shutting_down() const;

    void set_absolute_movable(DanglingBaseClassRef<IAbsoluteMovable> absolute_movable);
    IAbsoluteMovable& get_absolute_movable() const;
    bool has_absolute_movable() const;
    void clear_absolute_movable();

    IRelativeMovable& get_relative_movable() const;
    void set_relative_movable(const observer_ptr<IRelativeMovable, SceneNode&>& relative_movable);
    bool has_relative_movable() const;
    void clear_relative_movable();

    INodeModifier& get_node_modifier() const;
    bool has_node_modifier() const;
    void set_node_modifier(std::unique_ptr<INodeModifier>&& node_modifier);

    bool has_absolute_observer() const;
    IAbsoluteObserver& get_absolute_observer() const;
    void set_absolute_observer(IAbsoluteObserver& absolute_observer);

    bool has_sticky_absolute_observer() const;

    IAbsoluteObserver& get_sticky_absolute_observer() const;
    void set_sticky_absolute_observer(IAbsoluteObserver& sticky_absolute_observer);

    void insert_node_hider(INodeHider& node_hider);
    void remove_node_hider(INodeHider& node_hider);
    void add_renderable(
        const VariableAndHash<std::string>& name,
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
    void clear_renderable_instance(const VariableAndHash<std::string>& name);
    void clear_absolute_observer();
    void clear_sticky_absolute_observer();
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
        const FixedArray<ScenePos, 3>& position,
        float yangle,
        BillboardId billboard_id);
    void optimize_instances_search_time(std::ostream& ostr) const;
    bool has_camera() const;
    void set_camera(std::unique_ptr<Camera>&& camera);
    DanglingBaseClassRef<Camera> get_camera(SourceLocation loc) const;
    DanglingBaseClassPtr<Camera> try_get_camera(SourceLocation loc) const;
    void add_light(std::shared_ptr<Light> light);
    void add_skidmark(std::shared_ptr<Skidmark> skidmark);
    bool visit_all(
        const TransformationMatrix<float, ScenePos, 3>& parent_m,
        const std::function<bool(
            const TransformationMatrix<float, ScenePos, 3>& m,
            const std::unordered_map<VariableAndHash<std::string>, std::shared_ptr<RenderableWithStyle>>& renderables)>& func) const;
    void move(
        const TransformationMatrix<float, ScenePos, 3>& v,
        float dt,
        std::chrono::steady_clock::time_point time,
        SceneNodeResources* scene_node_resources,
        const AnimationState* animation_state);
    RenderingStrategies rendering_strategies() const;
    bool requires_render_pass(ExternalRenderPassType render_pass) const;
    void render(
        const FixedArray<ScenePos, 4, 4>& parent_mvp,
        const TransformationMatrix<float, ScenePos, 3>& parent_m,
        const TransformationMatrix<float, ScenePos, 3>& iv,
        const DanglingPtr<const SceneNode>& camera_node,
        const IDynamicLights* dynamic_lights,
        const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>>& lights,
        const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>>& skidmarks,
        std::list<Blended>& blended,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        const ExternalRenderPass& external_render_pass,
        const std::shared_ptr<const AnimationState>& animation_state,
        const std::list<const ColorStyle*>& color_styles,
        SceneNodeVisibility visibility = SceneNodeVisibility::VISIBLE) const;
    void append_sorted_aggregates_to_queue(
        const FixedArray<ScenePos, 4, 4>& parent_mvp,
        const TransformationMatrix<float, ScenePos, 3>& parent_m,
        const FixedArray<ScenePos, 3>& offset,
        std::list<std::pair<float, std::shared_ptr<ColoredVertexArray<float>>>>& aggregate_queue,
        const SceneGraphConfig& scene_graph_config,
        const ExternalRenderPass& external_render_pass) const;
    void append_large_aggregates_to_queue(
        const TransformationMatrix<float, ScenePos, 3>& parent_m,
        const FixedArray<ScenePos, 3>& offset,
        std::list<std::shared_ptr<ColoredVertexArray<float>>>& aggregate_queue,
        const SceneGraphConfig& scene_graph_config) const;
    void append_small_instances_to_queue(
        const FixedArray<ScenePos, 4, 4>& parent_mvp,
        const TransformationMatrix<float, ScenePos, 3>& parent_m,
        const TransformationMatrix<float, ScenePos, 3>& iv,
        const FixedArray<ScenePos, 3>& offset,
        const PositionAndYAngleAndBillboardId<CompressedScenePos>& delta_pose,
        SmallInstancesQueues& instances_queues,
        const SceneGraphConfig& scene_graph_config) const;
    void append_large_instances_to_queue(
        const FixedArray<ScenePos, 4, 4>& parent_mvp,
        const TransformationMatrix<float, ScenePos, 3>& parent_m,
        const FixedArray<ScenePos, 3>& offset,
        const PositionAndYAngleAndBillboardId<CompressedScenePos>& delta_pose,
        LargeInstancesQueue& instances_queue,
        const SceneGraphConfig& scene_graph_config) const;
    void append_lights_to_queue(
        const TransformationMatrix<float, ScenePos, 3>& parent_m,
        std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>>& lights) const;
    void append_skidmarks_to_queue(
        const TransformationMatrix<float, ScenePos, 3>& parent_m,
        std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>>& skidmarks) const;
    void append_static_filtered_to_queue(
        const TransformationMatrix<float, ScenePos, 3>& parent_m,
        std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<ColoredVertexArray<float>>>>& float_queue,
        std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<ColoredVertexArray<double>>>>& double_queue,
        const ColoredVertexArrayFilter& filter) const;
    const FixedArray<ScenePos, 3>& position() const;
    FixedArray<float, 3> rotation() const;
    float scale() const;
    void set_position(
        const FixedArray<ScenePos, 3>& position,
        std::optional<std::chrono::steady_clock::time_point> time);
    void set_rotation(
        const FixedArray<float, 3>& rotation,
        std::optional<std::chrono::steady_clock::time_point> time);
    void set_scale(float scale);
    void set_relative_pose(
        const FixedArray<ScenePos, 3>& position,
        const FixedArray<float, 3>& rotation,
        float scale,
        std::optional<std::chrono::steady_clock::time_point> time);
    void set_absolute_pose(
        const FixedArray<ScenePos, 3>& position,
        const FixedArray<float, 3>& rotation,
        float scale,
        std::optional<std::chrono::steady_clock::time_point> time);
    TransformationMatrix<float, ScenePos, 3> relative_model_matrix(std::chrono::steady_clock::time_point time = std::chrono::steady_clock::time_point()) const;
    TransformationMatrix<float, ScenePos, 3> absolute_model_matrix(std::chrono::steady_clock::time_point time = std::chrono::steady_clock::time_point()) const;
    TransformationMatrix<float, ScenePos, 3> relative_view_matrix(std::chrono::steady_clock::time_point time = std::chrono::steady_clock::time_point()) const;
    TransformationMatrix<float, ScenePos, 3> absolute_view_matrix(std::chrono::steady_clock::time_point time = std::chrono::steady_clock::time_point()) const;
    Bijection<TransformationMatrix<float, ScenePos, 3>> relative_bijection(
        std::chrono::steady_clock::time_point time = std::chrono::steady_clock::time_point()) const;
    Bijection<TransformationMatrix<float, ScenePos, 3>> absolute_bijection(
        std::chrono::steady_clock::time_point time = std::chrono::steady_clock::time_point()) const;
    FixedArray<float, 3> velocity(
        std::chrono::steady_clock::time_point time,
        std::chrono::steady_clock::duration dt) const;
    std::optional<AxisAlignedBoundingBox<ScenePos, 3>> relative_aabb() const;
    BoundingSphere<ScenePos, 3> relative_bounding_sphere() const;
    ScenePos max_center_distance(BillboardId billboard_id) const;
    void print(std::ostream& ostr, size_t recursion_depth = 0) const;
    bool has_color_style(const VariableAndHash<std::string>& name) const;
    ColorStyle& color_style(const VariableAndHash<std::string>& name);
    const ColorStyle& color_style(const VariableAndHash<std::string>& name) const;
    void add_color_style(std::unique_ptr<ColorStyle>&& color_style);
    void set_animation_state(std::unique_ptr<AnimationState>&& animation_state);
    void set_animation_state_updater(std::unique_ptr<AnimationStateUpdater>&& animation_state_updater);
    bool to_be_deleted() const;
    void set_bone(const SceneNodeBone& bone);
    void set_periodic_animation(const std::string& name);
    void set_aperiodic_animation(const std::string& name);
    void set_scene_and_state(Scene& scene, SceneNodeState state);
    SceneNodeState state() const;
    Scene& scene();
    const Scene& scene() const;
    void set_debug_message(std::string message);
    std::string debug_message() const;
    PoseInterpolationMode pose_interpolation_mode() const;
    void invalidate_transformation_history();
    mutable DestructionObservers<SceneNode&> clearing_observers;
    mutable DestructionObservers<SceneNode&> destruction_observers;
    mutable SharedPtrs clearing_pointers;
    mutable SharedPtrs destruction_pointers;
    mutable DestructionFunctions on_clear;
    mutable DestructionFunctions on_destroy;
private:
    void set_scene_and_state_unsafe(Scene& scene, SceneNodeState state);
    void setup_child_unsafe(
        const std::string& name,
        DanglingRef<SceneNode> node,
        ChildRegistrationState child_registration_state,
        ChildParentState child_parent_state);
    void clear_unsafe();
    TransformationMatrix<float, ScenePos, 3> absolute_model_matrix(
        LockingStrategy locking_strategy,
        std::chrono::steady_clock::time_point time = std::chrono::steady_clock::time_point()) const;
    Scene* scene_;
    DanglingPtr<SceneNode> parent_;
    DanglingBaseClassPtr<IAbsoluteMovable> absolute_movable_;
    DanglingBaseClassPtr<IRelativeMovable> relative_movable_;
    std::unique_ptr<INodeModifier> node_modifier_;
    std::set<INodeHider*> node_hiders_;
    IAbsoluteObserver* absolute_observer_;
    IAbsoluteObserver* sticky_absolute_observer_;
    std::unique_ptr<Camera> camera_;
    std::unordered_map<VariableAndHash<std::string>, std::shared_ptr<RenderableWithStyle>> renderables_;
    std::map<std::string, SceneNodeChild> children_;
    std::map<std::string, SceneNodeChild> aggregate_children_;
    std::map<std::string, SceneNodeInstances> instances_children_;
    std::list<std::shared_ptr<Light>> lights_;
    std::list<std::shared_ptr<Skidmark>> skidmarks_;
    OffsetAndQuaternion<float, ScenePos> trafo_;
    QuaternionSeries<float, ScenePos, NINTERPOLATED> trafo_history_;
    bool trafo_history_invalidated_;
    float scale_;
    FixedArray<float, 3, 3> rotation_matrix_;
    PoseInterpolationMode interpolation_mode_;
    std::shared_ptr<AnimationState> animation_state_;
    std::list<std::unique_ptr<ColorStyle>> color_styles_;
    std::unique_ptr<AnimationStateUpdater> animation_state_updater_;
    SceneNodeBone bone_;
    std::string periodic_animation_;
    std::string aperiodic_animation_;
    SceneNodeState state_;
    mutable SafeAtomicRecursiveSharedMutex mutex_;
    mutable SafeAtomicRecursiveSharedMutex pose_mutex_;
    std::atomic_bool shutting_down_;
    std::atomic_bool shutdown_called_;
    std::string debug_message_;
};

std::ostream& operator << (std::ostream& ostr, DanglingPtr<const SceneNode> node);

}
