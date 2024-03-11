#include "Scene_Node.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation/Quaternion.hpp>
#include <Mlib/Math/Transformation/Tait_Bryan_Angles.hpp>
#include <Mlib/Memory/Recursive_Deletion.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene_Graph/Animation/Animation_State_Updater.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Animation_State.hpp>
#include <Mlib/Scene_Graph/Elements/Color_Style.hpp>
#include <Mlib/Scene_Graph/Elements/Light.hpp>
#include <Mlib/Scene_Graph/Elements/Renderable.hpp>
#include <Mlib/Scene_Graph/Elements/Skidmark.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/IAbsolute_Movable.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/IAbsolute_Observer.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/INode_Hider.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/INode_Modifier.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/IRelative_Movable.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <Mlib/Scene_Graph/Render_Pass_Extended.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

PoseInterpolationMode Mlib::pose_interpolation_mode_from_string(const std::string& s) {
    if (s == "disabled") {
        return PoseInterpolationMode::DISABLED;
    } else if (s == "enabled") {
        return PoseInterpolationMode::ENABLED;
    } else {
        THROW_OR_ABORT("Unknown pose interpolation mode: \"" + s + '"');
    }
}

SceneNode::SceneNode(
    const FixedArray<double, 3>& position,
    const FixedArray<float, 3>& rotation,
    float scale,
    PoseInterpolationMode interpolation_mode)
    : clearing_observers{DanglingRef<SceneNode>::from_object(*this, DP_LOC)}
    , destruction_observers{DanglingRef<SceneNode>::from_object(*this, DP_LOC)}
    , scene_{ nullptr }
    , parent_{ nullptr }
    , absolute_movable_{ nullptr }
    , relative_movable_{ nullptr }
    , absolute_observer_{ nullptr }
    , sticky_absolute_observer_{ nullptr }
    , trafo_{ OffsetAndQuaternion<float, double>::from_tait_bryan_angles({ rotation, position }) }
    , trafo_history_{ trafo_, std::chrono::steady_clock::now() }
    , trafo_history_invalidated_{ false }
    , scale_{ scale }
    , rotation_matrix_{ tait_bryan_angles_2_matrix(rotation) }
    , interpolation_mode_{interpolation_mode}
    , state_{ SceneNodeState::DETACHED }
    , shutting_down_{ false }
{}

SceneNode::SceneNode(PoseInterpolationMode interpolation_mode)
: SceneNode{
    fixed_zeros<double, 3>(),
    fixed_zeros<float, 3>(),
    1.f,
    interpolation_mode}
{}

SceneNode::~SceneNode() {
    if (state_ == SceneNodeState::STATIC) {
        if (scene_ == nullptr) {
            verbose_abort("ERROR: Scene is null in static node");
        }
        if (!scene_->shutting_down()) {
            verbose_abort("ERROR: Static node is being deleted but scene not shutting down");
        }
    }
    shutting_down_ = true;
    sticky_absolute_observer_ = nullptr;
    destruction_observers.shutdown();
    destruction_pointers.clear();
    clear_unsafe();
}

bool SceneNode::shutting_down() const {
    // The destruction order is specified explicitly
    // in the SceneNode destructor.
    return shutting_down_;
}

void SceneNode::set_parent(DanglingRef<SceneNode> parent) {
    std::scoped_lock lock{ mutex_ };
    if (has_parent()) {
        THROW_OR_ABORT("Node already has a parent");
    }
    parent_ = parent.ptr();
    parent_.set_loc(DP_LOC);
}

bool SceneNode::has_parent() const {
    std::shared_lock lock{ mutex_ };
    return (parent_ != nullptr);
}

DanglingRef<SceneNode> SceneNode::parent() {
    std::shared_lock lock{ mutex_ };
    if (!has_parent()) {
        THROW_OR_ABORT("Node has no parent");
    }
    return *parent_;
}

DanglingRef<const SceneNode> SceneNode::parent() const {
    return const_cast<SceneNode*>(this)->parent();
}

void SceneNode::setup_child_unsafe(
    const std::string& name,
    DanglingRef<SceneNode> node,
    ChildRegistrationState child_registration_state,
    ChildParentState child_parent_state)
{
    // Required in SceneNonde::~SceneNode
    if ((child_registration_state == ChildRegistrationState::REGISTERED) && (scene_ == nullptr)) {
        THROW_OR_ABORT("Parent of registered node " + name + " does not have a scene");
    }
    if (name.empty()) {
        THROW_OR_ABORT("Child node has no name");
    }
    if (child_parent_state == ChildParentState::PARENT_NOT_SET) {
        if (node->parent_ != nullptr) {
            THROW_OR_ABORT("Scene node \"" + name + "\" already has a parent");
        }
        node->parent_ = DanglingPtr<SceneNode>::from_object(*this, DP_LOC);
    } else if (node->parent_ != DanglingPtr<SceneNode>::from_object(*this, DP_LOC)) {
        THROW_OR_ABORT("Child parent mismatch");
    }
    if ((scene_ != nullptr) != (state_ != SceneNodeState::DETACHED)) {
        THROW_OR_ABORT("Conflicting scene nullness and node state");
    }
    if (scene_ != nullptr) {
        node->set_scene_and_state_unsafe(*scene_, state_);
    }
}

INodeModifier& SceneNode::get_node_modifier() const {
    std::shared_lock lock{ mutex_ };
    if (node_modifier_ == nullptr) {
        THROW_OR_ABORT("Node modifier not set");
    }
    return *node_modifier_;
}

void SceneNode::set_node_modifier(std::unique_ptr<INodeModifier>&& node_modifier)
{
    std::scoped_lock lock{ mutex_ };
    if (node_modifier_ != nullptr) {
        THROW_OR_ABORT("Node modifier already set");
    }
    node_modifier_ = std::move(node_modifier);
}

void SceneNode::insert_node_hider(INodeHider& node_hider) {
    std::scoped_lock lock{ mutex_ };
    if (!node_hiders_.insert(&node_hider).second) {
        THROW_OR_ABORT("Node hider already inserted");
    }
}

void SceneNode::remove_node_hider(INodeHider& node_hider) {
    std::scoped_lock lock{ mutex_ };
    if (node_hiders_.erase(&node_hider) != 1) {
        THROW_OR_ABORT("Could not remove node hider");
    }
}

IAbsoluteMovable& SceneNode::get_absolute_movable() const {
    std::shared_lock lock{ mutex_ };
    if (absolute_movable_ == nullptr) {
        THROW_OR_ABORT("Absolute movable not set");
    }
    return *absolute_movable_;
}

bool SceneNode::has_absolute_movable() const {
    return (absolute_movable_ != nullptr);
}

IRelativeMovable& SceneNode::get_relative_movable() const {
    std::shared_lock lock{ mutex_ };
    if (relative_movable_ == nullptr) {
        THROW_OR_ABORT("Relative movable not set");
    }
    return *relative_movable_;
}

void SceneNode::set_relative_movable(const observer_ptr<IRelativeMovable, DanglingRef<const SceneNode>>& relative_movable)
{
    auto m = absolute_model_matrix();
    std::scoped_lock lock{ mutex_ };
    if (relative_movable_ != nullptr) {
        THROW_OR_ABORT("Relative movable already set");
    }
    relative_movable_ = relative_movable.get();
    relative_movable_->set_initial_relative_model_matrix(relative_model_matrix_unsafe());
    relative_movable_->set_absolute_model_matrix(m);
    if (relative_movable.observer() != nullptr) {
        clearing_observers.add(*relative_movable.observer());
    }
}

IAbsoluteObserver& SceneNode::get_absolute_observer() const {
    std::shared_lock lock{ mutex_ };
    if (absolute_observer_ == nullptr) {
        THROW_OR_ABORT("Absolute observer not set");
    }
    return *absolute_observer_;
}

IAbsoluteObserver& SceneNode::get_sticky_absolute_observer() const {
    std::shared_lock lock{ mutex_ };
    if (sticky_absolute_observer_ == nullptr) {
        THROW_OR_ABORT("Sticky absolute observer not set");
    }
    return *sticky_absolute_observer_;
}

void SceneNode::set_absolute_observer(const observer_ptr<IAbsoluteObserver, DanglingRef<const SceneNode>>& absolute_observer)
{
    auto m = absolute_model_matrix();
    std::scoped_lock lock{ mutex_ };
    if (absolute_observer_ != nullptr) {
        THROW_OR_ABORT("Absolute observer already set");
    }
    if (absolute_observer.observer() == nullptr) {
        THROW_OR_ABORT("Absolute destruction observer cannot be null");
    }
    absolute_observer_ = absolute_observer.get();
    absolute_observer_->set_absolute_model_matrix(m);
    clearing_observers.add(*absolute_observer.observer());
}

void SceneNode::set_sticky_absolute_observer(const observer_ptr<IAbsoluteObserver, DanglingRef<const SceneNode>>& sticky_absolute_observer)
{
    auto m = absolute_model_matrix();
    std::scoped_lock lock{ mutex_ };
    if (sticky_absolute_observer_ != nullptr) {
        THROW_OR_ABORT("Sticky absolute observer already set");
    }
    if (sticky_absolute_observer.observer() == nullptr) {
        THROW_OR_ABORT("Absolute destruction observer cannot be null");
    }
    sticky_absolute_observer_ = sticky_absolute_observer.get();
    sticky_absolute_observer_->set_absolute_model_matrix(m);
    destruction_observers.add(*sticky_absolute_observer.observer());
}

void SceneNode::add_renderable(
    const std::string& name,
    const std::shared_ptr<const Renderable>& renderable)
{
    std::scoped_lock lock{ mutex_ };
    if (name.empty()) {
        THROW_OR_ABORT("Renderable has no name");
    }
    if (!renderables_.insert(std::make_pair(name, renderable)).second) {
        THROW_OR_ABORT("Renderable with name " + name + " already exists");
    }
}

bool SceneNode::has_node_modifier() const {
    std::shared_lock lock{ mutex_ };
    return node_modifier_ != nullptr;
}

void SceneNode::clear_renderable_instance(const std::string& name) {
    std::scoped_lock lock{ mutex_ };
    if (renderables_.erase(name) != 1) {
        THROW_OR_ABORT("Could not clear renderable with name \"" + name + '"');
    }
}

void SceneNode::clear_absolute_observer() {
    std::scoped_lock lock{ mutex_ };
    absolute_observer_ = nullptr;
}

void SceneNode::clear() {
    std::shared_lock lock{ mutex_ };
    if (shutting_down()) {
        verbose_abort("Node to be cleared is shutting down");
    }
    clear_unsafe();
}

void SceneNode::clear_unsafe() {
    clearing_observers.notify_destroyed();
    clearing_pointers.clear();

    absolute_movable_ = nullptr;
    relative_movable_ = nullptr;
    node_modifier_ = nullptr;
    node_hiders_.clear();
    absolute_observer_ = nullptr;
    camera_ = nullptr;
    renderables_.clear();
    clear_map_recursively(children_, [this](const auto& child){
        if (child.mapped().is_registered) {
            if (child.mapped().scene_node->shutting_down()) {
                verbose_abort("Child node \"" + child.key() + "\" already shutting down (0)");
            }
            // scene_ is non-null, checked in "add_child".
            scene_->unregister_node(child.key());
        }
    });
    clear_map_recursively(aggregate_children_, [this](const auto& child){
        if (child.mapped().is_registered) {
            if (child.mapped().scene_node->shutting_down()) {
                verbose_abort("Child node \"" + child.key() + "\" already shutting down (1)");
            }
            // scene_ is non-null, checked in "add_child".
            scene_->unregister_node(child.key());
        }
    });
    instances_children_.clear();
    lights_.clear();
    animation_state_ = nullptr;
    color_styles_.clear();
    animation_state_updater_ = nullptr;
    periodic_animation_.clear();
    aperiodic_animation_.clear();
}

void SceneNode::add_child(
    const std::string& name,
    DanglingUniquePtr<SceneNode>&& node,
    ChildRegistrationState child_registration_state,
    ChildParentState child_parent_state)
{
    std::scoped_lock lock{ mutex_ };
    setup_child_unsafe(name, node.ref(DP_LOC), child_registration_state, child_parent_state);
    if (!children_.insert(std::make_pair(name, SceneNodeChild{
        .is_registered = (child_registration_state == ChildRegistrationState::REGISTERED),
        .scene_node = std::move(node)})).second)
    {
        THROW_OR_ABORT("Child node with name " + name + " already exists");
    }
}

DanglingRef<SceneNode> SceneNode::get_child(const std::string& name) {
    std::shared_lock lock{ mutex_ };
    auto it = children_.find(name);
    if (it == children_.end()) {
        THROW_OR_ABORT("Node does not have a child with name \"" + name + '"');
    }
    return it->second.scene_node.ref(DP_LOC);
}

DanglingRef<const SceneNode> SceneNode::get_child(const std::string& name) const {
    return const_cast<SceneNode*>(this)->get_child(name);
}

void SceneNode::remove_child(const std::string& name) {
    std::scoped_lock lock{ mutex_ };
    if (state_ == SceneNodeState::STATIC) {
        verbose_abort("Cannot remove child \"" + name + "\" from static node");
    }
    auto it = children_.find(name);
    if (it == children_.end()) {
        verbose_abort("Cannot not remove child with name \"" + name + "\" because it does not exist");
    }
    if (it->second.is_registered) {
        if (scene_ == nullptr) {
            verbose_abort("Can not deregister child \"" + name + "\" because scene is not set");
        }
        if (it->second.scene_node->shutting_down()) {
            verbose_abort("Child node \"" + name + "\" shutting down (2)");
        }
        scene_->unregister_node(name);
    }
    children_.erase(it);
}

bool SceneNode::contains_child(const std::string& name) const {
    std::shared_lock lock{ mutex_ };
    return children_.find(name) != children_.end();
}

void SceneNode::add_aggregate_child(
    const std::string& name,
    DanglingUniquePtr<SceneNode>&& node,
    ChildRegistrationState child_registration_state,
    ChildParentState child_parent_state)
{
    std::scoped_lock lock{ mutex_ };
    setup_child_unsafe(name, node.ref(DP_LOC), child_registration_state, child_parent_state);
    if (!aggregate_children_.insert(std::make_pair(name, SceneNodeChild{
        .is_registered = (child_registration_state == ChildRegistrationState::REGISTERED),
        .scene_node = std::move(node)})).second)
    {
        THROW_OR_ABORT("Aggregate node with name " + name + " already exists");
    }
}

void SceneNode::add_instances_child(
    const std::string& name,
    DanglingUniquePtr<SceneNode>&& node,
    ChildRegistrationState child_registration_state,
    ChildParentState child_parent_state)
{
    std::scoped_lock lock{ mutex_ };
    setup_child_unsafe(name, node.ref(DP_LOC), child_registration_state, child_parent_state);
    if (!instances_children_.insert(std::make_pair(name, SceneNodeInstances{
        .is_registered = (child_registration_state == ChildRegistrationState::REGISTERED),
        .scene_node = std::move(node),
        .max_center_distance = 0.,
        .small_instances = Bvh<double, PositionAndYAngle, 3>({0.1, 0.1, 0.1}, 10),
        .large_instances = std::list<PositionAndYAngle>()})).second)
    {
        THROW_OR_ABORT("Instances node with name " + name + " already exists");
    }
}

void SceneNode::add_instances_position(
    const std::string& name,
    const FixedArray<double, 3>& position,
    float yangle,
    uint32_t billboard_id)
{
    std::scoped_lock lock{ mutex_ };
    auto cit = instances_children_.find(name);
    if (cit == instances_children_.end()) {
        THROW_OR_ABORT("Could not find instance node with name \"" + name + '"');
    }
    double mcd = cit->second.scene_node->max_center_distance(billboard_id);
    if (mcd == 0.) {
        THROW_OR_ABORT("Could not determine max_center_distance of node with name \"" + name + '"');
    }
    if (mcd == INFINITY) {
        cit->second.large_instances.push_back(
            PositionAndYAngle{
                .position = position,
                .yangle = yangle,
                .billboard_id = billboard_id}
        );
    } else {
        if (!std::isfinite(mcd)) {
            THROW_OR_ABORT("max_center_distance is not finite");
        }
        cit->second.max_center_distance = std::max(cit->second.max_center_distance, mcd);
        cit->second.small_instances.insert(
            position,
            PositionAndYAngle{
                .position = position,
                .yangle = yangle,
                .billboard_id = billboard_id});
    }
}

void SceneNode::optimize_instances_search_time(std::ostream& ostr) const {
    std::shared_lock lock{ mutex_ };
    for (const auto& [name, i] : instances_children_) {
        ostr << name << std::endl;
        i.small_instances.optimize_search_time(BvhDataRadiusType::ZERO, std::cerr);
        // i.small_instances.plot_svg<double>("/tmp/" + name + ".svg", 0, 1);
    }
}

bool SceneNode::has_camera() const {
    std::shared_lock lock{ mutex_ };
    return camera_ != nullptr;
}

void SceneNode::set_camera(std::unique_ptr<Camera>&& camera) {
    std::scoped_lock lock{ mutex_ };
    if (camera_ != nullptr) {
        THROW_OR_ABORT("Camera already set");
    }
    camera_ = std::move(camera);
}

Camera& SceneNode::get_camera() const {
    std::shared_lock lock{ mutex_ };
    if (camera_ == nullptr) {
        THROW_OR_ABORT("Node has no camera");
    }
    return *camera_.get();
}

void SceneNode::add_light(std::unique_ptr<Light>&& light) {
    std::scoped_lock lock{ mutex_ };
    lights_.push_back(std::move(light));
}

void SceneNode::add_skidmark(std::unique_ptr<Skidmark>&& skidmark) {
    std::scoped_lock lock{ mutex_ };
    skidmarks_.push_back(std::move(skidmark));
}

bool SceneNode::has_color_style(const std::string& name) const {
    std::shared_lock lock{ mutex_ };
    bool style_found = false;
    for (const auto& s : color_styles_) {
        if (!Mlib::re::regex_search(name, s->selector)) {
            continue;
        }
        if (style_found) {
            THROW_OR_ABORT("Node has multiple color styles matching \"" + name + '"');
        }
        style_found = true;
    }
    return style_found;
}

ColorStyle& SceneNode::color_style(const std::string& name) {
    return const_cast<ColorStyle&>(static_cast<const SceneNode*>(this)->color_style(name));
}

const ColorStyle& SceneNode::color_style(const std::string& name) const {
    std::shared_lock lock{ mutex_ };
    ColorStyle* result = nullptr;
    for (const auto& s : color_styles_) {
        if (!Mlib::re::regex_search(name, s->selector)) {
            continue;
        }
        if (result != nullptr) {
            THROW_OR_ABORT("Node has multiple color styles matching \"" + name + '"');
        }
        result = s.get();
    }
    if (result == nullptr) {
        THROW_OR_ABORT("Node has no style matching \"" + name + '"');
    }
    return *result;
}

void SceneNode::add_color_style(std::unique_ptr<ColorStyle>&& color_style) {
    std::scoped_lock lock{ mutex_ };
    if ((state_ != SceneNodeState::DETACHED) && !renderables_.empty()) {
        THROW_OR_ABORT(
            "Color style was set after renderables on a non-detached node. "
            "This leads to a race condition.");
    }
    color_styles_.push_back(std::move(color_style));
}

void SceneNode::set_animation_state(std::unique_ptr<AnimationState>&& animation_state) {
    std::scoped_lock lock{ mutex_ };
    if (!renderables_.empty()) {
        THROW_OR_ABORT("Animation state was set after renderables, this leads to a race condition");
    }
    if (animation_state_ != nullptr) {
        THROW_OR_ABORT("Scene node already has an animation state");
    }
    animation_state_ = std::move(animation_state);
}

void SceneNode::set_animation_state_updater(std::unique_ptr<AnimationStateUpdater>&& animation_state_updater) {
    std::scoped_lock lock{ mutex_ };
    if (!renderables_.empty()) {
        THROW_OR_ABORT("Animation state updater was set after renderables, this leads to a race condition");
    }
    if (animation_state_updater_ != nullptr) {
        THROW_OR_ABORT("Scene node already has an animation state updater");
    }
    animation_state_updater_ = std::move(animation_state_updater);
}

void SceneNode::move(
    const TransformationMatrix<float, double, 3>& v,
    float dt,
    std::chrono::steady_clock::time_point time,
    SceneNodeResources* scene_node_resources,
    const AnimationState* animation_state)
{
    std::scoped_lock lock{ mutex_ };
    if (state_ == SceneNodeState::STATIC) {
        THROW_OR_ABORT("Cannot move static node");
    }
    if (node_modifier_ != nullptr) {
        node_modifier_->modify_node();
    }
    const AnimationState* estate = animation_state_ != nullptr
        ? animation_state_.get()
        : animation_state;
    if (!bone_.name.empty()) {
        if (estate == nullptr) {
            THROW_OR_ABORT("Bone name is not empty, but animation state is not set");
        }
        auto apply_scene_node_animation = [&](const AnimationFrame& animation_frame, const std::string& animation_name){
            if (animation_name.empty() || animation_name == "<no_animation>") {
                return;
            }
            if (scene_node_resources == nullptr) {
                THROW_OR_ABORT("Scene node animation without scene node resources");
            }
            if (std::isnan(animation_frame.time)) {
                THROW_OR_ABORT("Scene node animation loop time is NAN");
            }
            auto poses = scene_node_resources->get_absolute_poses(
                animation_name,
                animation_frame.time);
            auto it = poses.find(bone_.name);
            if (it == poses.end()) {
                THROW_OR_ABORT("Could not find bone with name \"node\" in animation \"" + animation_name + '"');
            }
            OffsetAndQuaternion<float, double> q1{it->second.offset().casted<double>(), it->second.quaternion()};
            auto res_pose = trafo_.slerp(q1, 1.f - bone_.smoothness);
            res_pose.quaternion() = res_pose.quaternion().slerp(Quaternion<float>::identity(), 1.f - bone_.rotation_strength);
            set_relative_pose(
                res_pose.offset(),
                res_pose.quaternion().to_tait_bryan_angles(),
                scale(),
                SUCCESSOR_POSE);
        };
        if (estate->aperiodic_animation_frame.active()) {
            apply_scene_node_animation(
                estate->aperiodic_animation_frame.frame,
                aperiodic_animation_.empty()
                    ? estate->aperiodic_skelletal_animation_name
                    : aperiodic_animation_);
        } else {
            apply_scene_node_animation(
                estate->periodic_skelletal_animation_frame.frame,
                periodic_animation_.empty()
                    ? estate->periodic_skelletal_animation_name
                    : periodic_animation_);
        }
    }
    if (animation_state_ != nullptr) {
        if (animation_state_->aperiodic_animation_frame.active()) {
            animation_state_->aperiodic_animation_frame.advance_time(dt);
        } else {
            animation_state_->periodic_skelletal_animation_frame.advance_time(dt);
        }
        if (animation_state_updater_ != nullptr) {
            animation_state_updater_->update_animation_state(animation_state_.get());
        }
    }
    TransformationMatrix<float, double, 3> v2;
    if ((absolute_movable_ != nullptr) && (relative_movable_ != nullptr)) {
        auto ma = absolute_movable_->get_new_absolute_model_matrix();
        auto mr = v * ma;
        relative_movable_->set_updated_relative_model_matrix(mr);
        relative_movable_->set_absolute_model_matrix(ma);
        auto mr2 = relative_movable_->get_new_relative_model_matrix();
        set_relative_pose(mr2.t(), matrix_2_tait_bryan_angles(mr2.R()), 1.f, SUCCESSOR_POSE);
        v2 = relative_view_matrix() * v;
        absolute_movable_->set_absolute_model_matrix(v2.inverted_scaled());
    } else {
        if (absolute_movable_ != nullptr) {
            auto m = absolute_movable_->get_new_absolute_model_matrix();
            m = v * m;
            set_relative_pose(m.t(), matrix_2_tait_bryan_angles(m.R()), 1, SUCCESSOR_POSE);
        }
        v2 = relative_view_matrix() * v;
        if (relative_movable_ != nullptr) {
            relative_movable_->set_absolute_model_matrix(v2.inverted_scaled());
            auto m = relative_movable_->get_new_relative_model_matrix();
            set_relative_pose(m.t(), matrix_2_tait_bryan_angles(m.R()), 1, SUCCESSOR_POSE);
            v2 = relative_view_matrix() * v;
        }
    }
    if (absolute_observer_ != nullptr) {
        absolute_observer_->set_absolute_model_matrix(v2.inverted_scaled());
    }
    if (sticky_absolute_observer_ != nullptr) {
        sticky_absolute_observer_->set_absolute_model_matrix(v2.inverted_scaled());
    }
    for (auto it = children_.begin(); it != children_.end(); ) {
        it->second.scene_node->move(v2, dt, time, scene_node_resources, estate);
        if (it->second.scene_node->to_be_deleted()) {
            remove_child((it++)->first);
        } else {
            ++it;
        }
    }
    if ((interpolation_mode_ == PoseInterpolationMode::DISABLED) ||
        trafo_history_invalidated_)
    {
        trafo_history_.clear();
        trafo_history_invalidated_ = false;
    }
    trafo_history_.append(trafo_, time);
}

bool SceneNode::to_be_deleted() const {
    std::shared_lock lock{ mutex_ };
    return
        (animation_state_ != nullptr) &&
        animation_state_->delete_node_when_aperiodic_animation_finished &&
        animation_state_->aperiodic_animation_frame.ran_to_completion();
}

void SceneNode::set_bone(const SceneNodeBone& bone) {
    std::scoped_lock lock{ mutex_ };
    bone_ = bone;
}

void SceneNode::set_periodic_animation(const std::string& name) {
    std::scoped_lock lock{ mutex_ };
    periodic_animation_ = name;
}

void SceneNode::set_aperiodic_animation(const std::string& name) {
    std::scoped_lock lock{ mutex_ };
    aperiodic_animation_ = name;
}

void SceneNode::visit(
    const TransformationMatrix<float, double, 3>& parent_m,
    const std::function<void(
        const TransformationMatrix<float, double, 3>& m,
        const std::map<std::string, std::shared_ptr<const Renderable>>& renderables)>& func) const
{
    std::shared_lock lock{ mutex_ };
    scene_->delete_node_mutex().notify_reading();
    auto m = parent_m * relative_model_matrix_unsafe();
    func(m, renderables_);
    for (const auto& [_, n] : children_) {
        n.scene_node->visit(m, func);
    }
}

bool SceneNode::requires_render_pass(ExternalRenderPassType render_pass) const {
    std::shared_lock lock{ mutex_ };
    for (const auto& [_, r] : renderables_) {
        if (r->requires_render_pass(render_pass)) {
            return true;
        }
    }
    for (const auto& [_, n] : children_) {
        if (n.scene_node->requires_render_pass(render_pass)) {
            return true;
        }
    }
    return false;
}

void SceneNode::render(
    const FixedArray<double, 4, 4>& parent_mvp,
    const TransformationMatrix<float, double, 3>& parent_m,
    const TransformationMatrix<float, double, 3>& iv,
    DanglingRef<const SceneNode> camera_node,
    const std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
    const std::list<std::pair<TransformationMatrix<float, double, 3>, Skidmark*>>& skidmarks,
    std::list<Blended>& blended,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    const ExternalRenderPass& external_render_pass,
    const AnimationState* animation_state,
    const std::list<const ColorStyle*>& color_styles,
    SceneNodeVisibility visibility) const
{
    std::shared_lock lock{ mutex_ };
    if (state_ == SceneNodeState::DETACHED) {
        THROW_OR_ABORT("Cannot render detached node");
    }
    for (const auto& nh : node_hiders_) {
        // Note that the INodeHider may depend on this function being called,
        // so there should not be any additional check above this line.
        if (nh->node_shall_be_hidden(camera_node, external_render_pass)) {
            visibility = SceneNodeVisibility::INVISIBLE;
        }
    }
    // OpenGL matrices are transposed in memory,
    // https://stackoverflow.com/a/17718408/2292832.
    // "Note that post-multiplying with column-major matrices
    // produces the same result as pre-multiplying with
    // row-major matrices."
    auto cm = relative_model_matrix_unsafe(external_render_pass.time);
    FixedArray<double, 4, 4> mvp = dot2d(parent_mvp, cm.affine());
    auto m = parent_m * cm;
    const AnimationState* estate = animation_state_ != nullptr
        ? animation_state_.get()
        : animation_state;
    std::list<const ColorStyle*> ecolor_styles = color_styles;
    for (const auto& s : color_styles_) {
        ecolor_styles.push_back(s.get());
    }
    if (visibility == SceneNodeVisibility::VISIBLE) {
        for (const auto& [n, r] : renderables_) {
            ColorStyle r_style;
            for (const auto& style : ecolor_styles) {
                if (Mlib::re::regex_search(n, style->selector)) {
                    r_style.insert(*style);
                }
            }
            if (r->requires_render_pass(external_render_pass.pass)) {
                r->render(
                    mvp,
                    m,
                    iv,
                    lights,
                    skidmarks,
                    scene_graph_config,
                    render_config,
                    {external_render_pass, InternalRenderPass::INITIAL},
                    estate,
                    &r_style);
            }
            if (r->requires_blending_pass(external_render_pass.pass)) {
                blended.push_back(Blended{
                    .z_order = r->continuous_blending_z_order(),
                    .mvp = mvp,
                    .m = m,
                    .renderable = r.get(),
                    .animation_state = estate,
                    .color_style = std::move(r_style)});
            }
        }
    }
    for (const auto& [_, c] : children_) {
        c.scene_node->render(
            mvp,
            m,
            iv,
            camera_node,
            lights,
            skidmarks,
            blended,
            render_config,
            scene_graph_config,
            external_render_pass,
            estate,
            ecolor_styles,
            visibility);
    }
}

void SceneNode::append_sorted_aggregates_to_queue(
    const FixedArray<double, 4, 4>& parent_mvp,
    const TransformationMatrix<float, double, 3>& parent_m,
    const FixedArray<double, 3>& offset,
    std::list<std::pair<float, std::shared_ptr<ColoredVertexArray<float>>>>& aggregate_queue,
    const SceneGraphConfig& scene_graph_config,
    const ExternalRenderPass& external_render_pass) const
{
    std::shared_lock lock{ mutex_ };
    if (state_ != SceneNodeState::STATIC) {
        THROW_OR_ABORT("Cannot append sorted aggregates to queue for a non-static node");
    }
    // OpenGL matrices are transposed in memory,
    // https://stackoverflow.com/a/17718408/2292832.
    // "Note that post-multiplying with column-major matrices
    // produces the same result as pre-multiplying with
    // row-major matrices."
    auto cm = relative_model_matrix_unsafe(external_render_pass.time);
    FixedArray<double, 4, 4> mvp = dot2d(parent_mvp, cm.affine());
    auto m = parent_m * cm;
    for (const auto& [_, r] : renderables_) {
        r->append_sorted_aggregates_to_queue(mvp, m, offset, scene_graph_config, external_render_pass, aggregate_queue);
    }
    for (const auto& [_, c] : children_) {
        c.scene_node->append_sorted_aggregates_to_queue(mvp, m, offset, aggregate_queue, scene_graph_config, external_render_pass);
    }
    for (const auto& [_, a] : aggregate_children_) {
        a.scene_node->append_sorted_aggregates_to_queue(mvp, m, offset, aggregate_queue, scene_graph_config, external_render_pass);
    }
}

void SceneNode::append_large_aggregates_to_queue(
    const TransformationMatrix<float, double, 3>& parent_m,
    const FixedArray<double, 3>& offset,
    std::list<std::shared_ptr<ColoredVertexArray<float>>>& aggregate_queue,
    const SceneGraphConfig& scene_graph_config) const
{
    std::shared_lock lock{ mutex_ };
    if (state_ != SceneNodeState::STATIC) {
        THROW_OR_ABORT("Cannot append large aggregates to queue for a non-static node");
    }
    TransformationMatrix<float, double, 3> m = parent_m * relative_model_matrix_unsafe();
    for (const auto& [_, r] : renderables_) {
        r->append_large_aggregates_to_queue(m, offset, scene_graph_config, aggregate_queue);
    }
    for (const auto& [_, c] : children_) {
        c.scene_node->append_large_aggregates_to_queue(m, offset, aggregate_queue, scene_graph_config);
    }
    for (const auto& [_, a] : aggregate_children_) {
        a.scene_node->append_large_aggregates_to_queue(m, offset, aggregate_queue, scene_graph_config);
    }
}

void SceneNode::append_small_instances_to_queue(
    const FixedArray<double, 4, 4>& parent_mvp,
    const TransformationMatrix<float, double, 3>& parent_m,
    const TransformationMatrix<float, double, 3>& iv,
    const FixedArray<double, 3>& offset,
    const PositionAndYAngle& delta_pose,
    SmallInstancesQueues& instances_queues,
    const SceneGraphConfig& scene_graph_config) const
{
    std::shared_lock lock{ mutex_ };
    if (state_ != SceneNodeState::STATIC) {
        THROW_OR_ABORT("Cannot append small instances to queue for a non-static node");
    }
    TransformationMatrix<float, double, 3> rel = relative_model_matrix_unsafe();
    rel.t() += delta_pose.position;
    if (delta_pose.yangle != 0) {
        rel.R() = dot2d(rel.R(), rodrigues2(FixedArray<float, 3>{0.f, 1.f, 0.f}, delta_pose.yangle));
    }
    FixedArray<double, 4, 4> mvp = dot2d(parent_mvp, rel.affine());
    TransformationMatrix<float, double, 3> m = parent_m * rel;
    for (const auto& [_, r] : renderables_) {
        r->append_sorted_instances_to_queue(mvp, m, iv, offset, delta_pose.billboard_id, scene_graph_config, instances_queues);
    }
    for (const auto& [_, c] : children_) {
        c.scene_node->append_small_instances_to_queue(mvp, m, iv, offset, PositionAndYAngle{fixed_zeros<double, 3>(), 0.f, UINT32_MAX}, instances_queues, scene_graph_config);
    }
    for (const auto& [_, i] : instances_children_) {
        // The transformation is swapped, meaning
        // y = P * V * M * INSTANCE * NODE * x.
        for (const auto& j : i.large_instances) {
            i.scene_node->append_small_instances_to_queue(mvp, m, iv, offset, j, instances_queues, scene_graph_config);
        }
        if (!i.small_instances.empty()) {
            auto camera_position = m.inverted_scaled().transform(iv.t());
            i.small_instances.visit(
                AxisAlignedBoundingBox<double, 3>{camera_position, i.max_center_distance},
                [&, &i=i](const PositionAndYAngle& j){
                    i.scene_node->append_small_instances_to_queue(mvp, m, iv, offset, j, instances_queues, scene_graph_config);
                    return true;
                });
        }
    }
}

void SceneNode::append_large_instances_to_queue(
    const FixedArray<double, 4, 4>& parent_mvp,
    const TransformationMatrix<float, double, 3>& parent_m,
    const FixedArray<double, 3>& offset,
    const PositionAndYAngle& delta_pose,
    LargeInstancesQueue& instances_queue,
    const SceneGraphConfig& scene_graph_config) const
{
    std::shared_lock lock{ mutex_ };
    if (state_ != SceneNodeState::STATIC) {
        THROW_OR_ABORT("Cannot append large instances to queue for a non-static node");
    }
    TransformationMatrix<float, double, 3> rel = relative_model_matrix_unsafe();
    rel.t() += delta_pose.position;
    if (delta_pose.yangle != 0) {
        rel.R() = dot2d(rel.R(), rodrigues2(FixedArray<float, 3>{0.f, 1.f, 0.f}, delta_pose.yangle));
    }
    FixedArray<double, 4, 4> mvp = dot2d(parent_mvp, rel.affine());
    TransformationMatrix<float, double, 3> m = parent_m * rel;
    for (const auto& [_, r] : renderables_) {
        r->append_large_instances_to_queue(mvp, m, offset, delta_pose.billboard_id, scene_graph_config, instances_queue);
    }
    for (const auto& [_, c] : children_) {
        c.scene_node->append_large_instances_to_queue(mvp, m, offset, PositionAndYAngle{fixed_zeros<double, 3>(), 0.f, UINT32_MAX}, instances_queue, scene_graph_config);
    }
    for (const auto& [_, i] : instances_children_) {
        // The transformation is swapped, meaning
        // y = P * V * M * INSTANCE * NODE * x.
        for (const auto& j : i.large_instances) {
            i.scene_node->append_large_instances_to_queue(mvp, m, offset, j, instances_queue, scene_graph_config);
        }
        i.small_instances.visit_all([&, &i=i](const auto& aabb, const PositionAndYAngle& j){
            i.scene_node->append_large_instances_to_queue(mvp, m, offset, j, instances_queue, scene_graph_config);
            return true;
        });
    }
}

void SceneNode::append_lights_to_queue(
    const TransformationMatrix<float, double, 3>& parent_m,
    std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights) const
{
    std::shared_lock lock{ mutex_ };
    TransformationMatrix<float, double, 3> m = parent_m * relative_model_matrix_unsafe();
    for (const auto& l : lights_) {
        lights.push_back(std::make_pair(m, l.get()));
    }
    for (const auto& [_, c] : children_) {
        c.scene_node->append_lights_to_queue(m, lights);
    }
}

void SceneNode::append_skidmarks_to_queue(
    const TransformationMatrix<float, double, 3>& parent_m,
    std::list<std::pair<TransformationMatrix<float, double, 3>, Skidmark*>>& skidmarks) const
{
    std::shared_lock lock{ mutex_ };
    TransformationMatrix<float, double, 3> m = parent_m * relative_model_matrix_unsafe();
    for (const auto& s : skidmarks_) {
        skidmarks.push_back(std::make_pair(m, s.get()));
    }
    for (const auto& [_, c] : children_) {
        c.scene_node->append_skidmarks_to_queue(m, skidmarks);
    }
}

const FixedArray<double, 3>& SceneNode::position() const {
    std::shared_lock lock{ mutex_ };
    return trafo_.offset();
}

FixedArray<float, 3> SceneNode::rotation() const {
    std::shared_lock lock{ mutex_ };
    return matrix_2_tait_bryan_angles(rotation_matrix_);
}

float SceneNode::scale() const {
    std::shared_lock lock{ mutex_ };
    return scale_;
}

void SceneNode::set_position(
    const FixedArray<double, 3>& position,
    std::optional<std::chrono::steady_clock::time_point> time)
{
    std::scoped_lock lock{ mutex_ };
    if (state_ == SceneNodeState::STATIC) {
        THROW_OR_ABORT("Cannot set position for a static node");
    }
    trafo_.offset() = position;
    if (!time.has_value()) {
        // Do nothing
    } else if (time.value() == std::chrono::steady_clock::time_point()) {
        trafo_history_.clear();
        trafo_history_.append(trafo_, std::chrono::steady_clock::now());
    } else {
        trafo_history_.append(trafo_, time.value());
    }
}

void SceneNode::set_rotation(
    const FixedArray<float, 3>& rotation,
    std::optional<std::chrono::steady_clock::time_point> time)
{
    std::scoped_lock lock{ mutex_ };
    if (state_ == SceneNodeState::STATIC) {
        THROW_OR_ABORT("Cannot set rotation for a static node");
    }
    trafo_.quaternion() = Quaternion<float>::from_tait_bryan_angles(rotation);
    rotation_matrix_ = tait_bryan_angles_2_matrix(rotation);
    if (!time.has_value()) {
        // Do nothing
    } else if (time.value() == std::chrono::steady_clock::time_point()) {
        trafo_history_.clear();
        trafo_history_.append(trafo_, std::chrono::steady_clock::now());
    } else {
        trafo_history_.append(trafo_, time.value());
    }
}

void SceneNode::set_scale(float scale)
{
    std::scoped_lock lock{ mutex_ };
    if (state_ == SceneNodeState::STATIC) {
        THROW_OR_ABORT("Cannot set scale for a static node");
    }
    scale_ = scale;
}

void SceneNode::set_relative_pose(
    const FixedArray<double, 3>& position,
    const FixedArray<float, 3>& rotation,
    float scale,
    std::optional<std::chrono::steady_clock::time_point> time)
{
    set_position(position, SUCCESSOR_POSE);
    set_rotation(rotation, SUCCESSOR_POSE);
    set_scale(scale);
    if (!time.has_value()) {
        // Do nothing
    } else if (time.value() == std::chrono::steady_clock::time_point()) {
        trafo_history_.clear();
        trafo_history_.append(trafo_, std::chrono::steady_clock::now());
    } else {
        trafo_history_.append(trafo_, time.value());
    }
}

TransformationMatrix<float, double, 3> SceneNode::relative_model_matrix_unsafe(std::chrono::steady_clock::time_point time) const {
    if (time == std::chrono::steady_clock::time_point()) {
        return TransformationMatrix{rotation_matrix_ * scale_, trafo_.offset()};
    } else {
        auto res = trafo_history_.get(time);
        return TransformationMatrix{res.quaternion().to_rotation_matrix() * scale_, res.offset()};
    }
}

TransformationMatrix<float, double, 3> SceneNode::relative_model_matrix() const {
    std::shared_lock lock{ mutex_ };
    return relative_model_matrix_unsafe();
}

TransformationMatrix<float, double, 3> SceneNode::absolute_model_matrix(std::chrono::steady_clock::time_point time) const {
    return absolute_model_matrix(LockingStrategy::ACQUIRE_LOCK, time);
}

TransformationMatrix<float, double, 3> SceneNode::absolute_model_matrix(
    LockingStrategy locking_strategy,
    std::chrono::steady_clock::time_point time) const
{
    std::shared_lock lock{ mutex_, std::defer_lock };
    if (locking_strategy == LockingStrategy::ACQUIRE_LOCK) {
        lock.lock();
    }
    if (state_ != SceneNodeState::DETACHED) {
        scene_->delete_node_mutex().notify_reading();
    }
    auto result = relative_model_matrix_unsafe(time);
    if (parent_ != nullptr) {
        if (locking_strategy == LockingStrategy::ACQUIRE_LOCK) {
            lock.unlock();
        }
        return parent_->absolute_model_matrix(time) * result;
    } else {
        return result;
    }
}

TransformationMatrix<float, double, 3> SceneNode::relative_view_matrix_unsafe(std::chrono::steady_clock::time_point time) const {
    if (time == std::chrono::steady_clock::time_point()) {
        return TransformationMatrix<float, double, 3>::inverse(rotation_matrix_ / scale_, trafo_.offset());
    } else {
        auto res = trafo_history_.get(time);
        return TransformationMatrix<float, double, 3>::inverse(res.quaternion().to_rotation_matrix() / scale_, res.offset());
    }
}

TransformationMatrix<float, double, 3> SceneNode::relative_view_matrix() const {
    std::shared_lock lock{ mutex_ };
    return relative_view_matrix_unsafe();
}

TransformationMatrix<float, double, 3> SceneNode::absolute_view_matrix(std::chrono::steady_clock::time_point time) const {
    std::shared_lock lock{ mutex_ };
    if (state_ != SceneNodeState::DETACHED) {
        scene_->delete_node_mutex().notify_reading();
    }
    auto result = relative_view_matrix_unsafe(time);
    if (parent_ != nullptr) {
        lock.unlock();
        return result * parent_->absolute_view_matrix(time);
    } else {
        return result;
    }
}

FixedArray<float, 3> SceneNode::velocity(
    std::chrono::steady_clock::time_point time,
    std::chrono::steady_clock::duration dt) const
{
    auto p0 = absolute_model_matrix(time - dt);
    auto p1 = absolute_model_matrix(time + dt);
    return (p1.t() - p0.t()).casted<float>() / (2.f * std::chrono::duration<float>{dt}.count() * s);
}

void SceneNode::set_absolute_pose(
    const FixedArray<double, 3>& position,
    const FixedArray<float, 3>& rotation,
    float scale,
    std::optional<std::chrono::steady_clock::time_point> time)
{
    std::scoped_lock lock{ mutex_ };
    if (parent_ == nullptr) {
        set_relative_pose(
            position,
            rotation,
            scale,
            time);
    } else {
        auto p_v = parent_->absolute_view_matrix();
        auto m = TransformationMatrix<float, double, 3>{
            tait_bryan_angles_2_matrix(rotation) * scale,
            position};
        auto rel_trafo = p_v * m;
        float rel_scale = rel_trafo.get_scale();
        set_relative_pose(
            rel_trafo.t(),
            matrix_2_tait_bryan_angles(rel_trafo.R() / rel_scale),
            rel_scale,
            time);
    }
}

std::optional<AxisAlignedBoundingBox<double, 3>> SceneNode::relative_aabb() const {
    std::shared_lock lock{ mutex_ };
    std::optional<AxisAlignedBoundingBox<double, 3>> result;
    if (!renderables_.empty()) {
        result = AxisAlignedBoundingBox<double, 3>();
    }
    for (const auto& [_, r] : renderables_) {
        result.value().extend(r->aabb());
    }
    for (const auto& [_, c] : children_) {
        auto cb = c.scene_node->relative_aabb();
        if (cb.has_value()) {
            auto m = c.scene_node->relative_model_matrix();
            if (!result.has_value()) {
                result = cb.value().transformed(m);
            } else {
                result.value().extend(cb.value().transformed(m));
            }
        }
    }
    return result;
}

double SceneNode::max_center_distance(uint32_t billboard_id) const {
    std::shared_lock lock{ mutex_ };
    double result = 0.;
    for (const auto& [_, r] : renderables_) {
        result = std::max(result, r->max_center_distance(billboard_id));
    }
    for (const auto& [_, c] : children_) {
        auto cb = c.scene_node->max_center_distance(UINT32_MAX);
        if (cb != 0.f) {
            auto m = c.scene_node->relative_model_matrix();
            if (any(m.t() != 0.)) {
                THROW_OR_ABORT("Detected node translation in max_center_distance");
            }
            result = std::max(result, cb);
        }
    }
    return result;
}

void SceneNode::print(std::ostream& ostr, size_t recursion_depth) const {
    std::shared_lock lock{ mutex_ };
    std::string ind0(3 * recursion_depth, '-');
    std::string ind1(3 * recursion_depth + 1, '-');
    std::string ind2(3 * recursion_depth + 2, '-');
    ostr << " " << ind0 << " Node\n";
    ostr << " " << ind1 << " Position " << position() << '\n';
    ostr << " " << ind1 << " Rotation " << rotation() << '\n';
    if (!renderables_.empty()) {
        ostr << " " << ind1 << " Renderables (" << renderables_.size() << ")\n";
        for (const auto& [n, _] : renderables_) {
            ostr << " " << ind2 << " " << n << '\n';
        }
    }
    if (!children_.empty()) {
        ostr << " " << ind1 << " Children (" << children_.size() << ")\n";
        for (const auto& [n, c] : children_) {
            ostr << " " << ind2 << " " << n << '\n';
            c.scene_node->print(ostr, recursion_depth + 1);
        }
    }
    if (!aggregate_children_.empty()) {
        ostr << " " << ind1 << " Aggregates (" << aggregate_children_.size() << ")\n";
        for (const auto& [n, c] : aggregate_children_) {
            ostr << " " << ind2 << " " << n << '\n';
            c.scene_node->print(ostr, recursion_depth + 1);
        }
    }
    if (!instances_children_.empty()) {
        ostr << " " << ind1 << " Instances (" << instances_children_.size() << ")\n";
        for (const auto& [n, c] : instances_children_) {
            ostr << " " << ind2 << " " << n <<
                " #small=" << c.small_instances.size() <<
                " #large=" << c.large_instances.size() << '\n';
            c.scene_node->print(ostr, recursion_depth + 1);
        }
    }
    ostr << " " << ind0 << " End\n";
}

void SceneNode::set_scene_and_state(Scene& scene, SceneNodeState state) {
    std::scoped_lock lock{ mutex_ };
    set_scene_and_state_unsafe(scene, state);
}

void SceneNode::set_scene_and_state_unsafe(Scene& scene, SceneNodeState state) {
    if (scene_ != nullptr) {
        THROW_OR_ABORT("Scene node already has a scene");
    }
    scene_ = &scene;
    if (state_ != SceneNodeState::DETACHED) {
        THROW_OR_ABORT("Node state already set");
    }
    if (state == SceneNodeState::DETACHED) {
        THROW_OR_ABORT("Cannot set node state to \"detached\"");
    }
    if ((state == SceneNodeState::STATIC) && (scene_ == nullptr)) {
        THROW_OR_ABORT("Scene is null in static node");
    }
    state_ = state;
    for (auto& [_, c] : children_) {
        c.scene_node->set_scene_and_state(scene, state);
    }
    for (auto& [_, c] : aggregate_children_) {
        c.scene_node->set_scene_and_state(scene, state);
    }
    for (auto& [_, c] : instances_children_) {
        c.scene_node->set_scene_and_state(scene, state);
    }
}

Scene& SceneNode::scene() {
    std::shared_lock lock{ mutex_ };
    if (scene_ == nullptr) {
        THROW_OR_ABORT("Scene not set");
    }
    return *scene_;
}

const Scene& SceneNode::scene() const {
    return const_cast<SceneNode*>(this)->scene();
}

void SceneNode::set_debug_message(std::string message) {
    std::scoped_lock lock{ mutex_ };
    debug_message_ = message;
}

std::string SceneNode::debug_message() const {
    std::shared_lock lock{ mutex_ };
    return debug_message_;
}

PoseInterpolationMode SceneNode::pose_interpolation_mode() const {
    std::shared_lock lock{ mutex_ };
    return interpolation_mode_;
}

void SceneNode::invalidate_transformation_history() {
    std::scoped_lock lock{ mutex_ };
    trafo_history_invalidated_ = true;
}

std::ostream& Mlib::operator << (std::ostream& ostr, DanglingPtr<const SceneNode> node) {
    node->print(ostr);
    return ostr;
}
