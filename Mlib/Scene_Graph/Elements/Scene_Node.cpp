#include "Scene_Node.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Quaternion.hpp>
#include <Mlib/Memory/Recursive_Deletion.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Scene_Graph/Animation/Animation_State_Updater.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Animation_State.hpp>
#include <Mlib/Scene_Graph/Elements/Color_Style.hpp>
#include <Mlib/Scene_Graph/Elements/Light.hpp>
#include <Mlib/Scene_Graph/Elements/Node_Hider.hpp>
#include <Mlib/Scene_Graph/Elements/Renderable.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <Mlib/Scene_Graph/Transformation/Absolute_Movable.hpp>
#include <Mlib/Scene_Graph/Transformation/Absolute_Observer.hpp>
#include <Mlib/Scene_Graph/Transformation/Node_Modifier.hpp>
#include <Mlib/Scene_Graph/Transformation/Relative_Movable.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

SceneNode::SceneNode()
: clearing_observers{*this},
  destruction_observers{*this},
  scene_{ nullptr },
  parent_{ nullptr },
  absolute_movable_{ nullptr },
  relative_movable_{ nullptr },
  absolute_observer_{ nullptr },
  absolute_destruction_observer_{ nullptr },
  position_{ 0.f, 0.f, 0.f },
  rotation_{ 0.f, 0.f, 0.f },
  scale_{ 1.f },
  rotation_matrix_{ fixed_identity_array<float, 3>() },
  state_{ SceneNodeState::DETACHED }
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
    destruction_observers.shutdown();
    clear();
}

bool SceneNode::shutting_down() const {
    std::shared_lock lock{mutex_};
    // The destruction order is specified explicitly
    // in the SceneNode destructor.
    return destruction_observers.shutting_down();
}

void SceneNode::set_parent(SceneNode& parent) {
    std::scoped_lock lock{mutex_};
    if (has_parent()) {
        THROW_OR_ABORT("Node already has a parent");
    }
    parent_ = &parent;
}

bool SceneNode::has_parent() const {
    std::shared_lock lock{mutex_};
    return (parent_ != nullptr);
}

SceneNode& SceneNode::parent() {
    std::shared_lock lock{mutex_};
    if (!has_parent()) {
        THROW_OR_ABORT("Node has no parent");
    }
    return *parent_;
}

const SceneNode& SceneNode::parent() const {
    return const_cast<SceneNode*>(this)->parent();
}

void SceneNode::setup_child(
    const std::string& name,
    SceneNode& node,
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
        if (node.parent_ != nullptr) {
            THROW_OR_ABORT("Scene node \"" + name + "\" already has a parent");
        }
        node.parent_ = this;
    } else if (node.parent_ != this) {
        THROW_OR_ABORT("Child parent mismatch");
    }
    if ((scene_ != nullptr) != (state_ != SceneNodeState::DETACHED)) {
        THROW_OR_ABORT("Conflicting scene nullness and node state");
    }
    if (scene_ != nullptr) {
        node.set_scene_and_state(*scene_, state_);
    }
}

NodeModifier& SceneNode::get_node_modifier() const {
    std::shared_lock lock{mutex_};
    if (node_modifier_ == nullptr) {
        THROW_OR_ABORT("Node modifier not set");
    }
    return *node_modifier_;
}

void SceneNode::set_node_modifier(std::unique_ptr<NodeModifier>&& node_modifier)
{
    std::scoped_lock lock{mutex_};
    if (node_modifier_ != nullptr) {
        THROW_OR_ABORT("Node modifier already set");
    }
    node_modifier_ = std::move(node_modifier);
}

void SceneNode::insert_node_hider(NodeHider& node_hider) {
    std::scoped_lock lock{mutex_};
    if (!node_hiders_.insert(&node_hider).second) {
        THROW_OR_ABORT("Node hider already inserted");
    }
}

void SceneNode::remove_node_hider(NodeHider& node_hider) {
    std::scoped_lock lock{mutex_};
    if (node_hiders_.erase(&node_hider) != 1) {
        THROW_OR_ABORT("Could not remove node hider");
    }
}

AbsoluteMovable& SceneNode::get_absolute_movable() const {
    std::shared_lock lock{mutex_};
    if (absolute_movable_ == nullptr) {
        THROW_OR_ABORT("Absolute movable not set");
    }
    return *absolute_movable_;
}

RelativeMovable& SceneNode::get_relative_movable() const {
    std::shared_lock lock{mutex_};
    if (relative_movable_ == nullptr) {
        THROW_OR_ABORT("Relative movable not set");
    }
    return *relative_movable_;
}

void SceneNode::set_relative_movable(const observer_ptr<RelativeMovable>& relative_movable)
{
    std::scoped_lock lock{mutex_};
    if (relative_movable_ != nullptr) {
        THROW_OR_ABORT("Relative movable already set");
    }
    relative_movable_ = relative_movable.get();
    relative_movable_->set_initial_relative_model_matrix(relative_model_matrix());
    relative_movable_->set_absolute_model_matrix(absolute_model_matrix());
    if (relative_movable.observer() != nullptr) {
        clearing_observers.add(*relative_movable.observer());
    }
}

AbsoluteObserver& SceneNode::get_absolute_observer() const {
    std::shared_lock lock{mutex_};
    if (absolute_observer_ == nullptr) {
        THROW_OR_ABORT("Absolute observer not set");
    }
    return *absolute_observer_;
}

void SceneNode::set_absolute_observer(const observer_ptr<AbsoluteObserver>& absolute_observer)
{
    std::scoped_lock lock{mutex_};
    if (absolute_observer_ != nullptr) {
        THROW_OR_ABORT("Absolute observer already set");
    }
    if (absolute_observer.observer() == nullptr) {
        THROW_OR_ABORT("Absolute destruction observer cannot be null");
    }
    absolute_observer_ = absolute_observer.get();
    absolute_observer_->set_absolute_model_matrix(absolute_model_matrix());
    clearing_observers.add(*absolute_observer.observer());

    absolute_destruction_observer_ = absolute_observer.observer();
}

void SceneNode::add_renderable(
    const std::string& name,
    const std::shared_ptr<const Renderable>& renderable)
{
    std::scoped_lock lock{mutex_};
    if (name.empty()) {
        THROW_OR_ABORT("Renderable has no name");
    }
    if (!renderables_.insert(std::make_pair(name, renderable)).second) {
        THROW_OR_ABORT("Renderable with name " + name + " already exists");
    }
}

bool SceneNode::has_node_modifier() const {
    std::shared_lock lock{mutex_};
    return node_modifier_ != nullptr;
}

void SceneNode::clear_renderable_instance(const std::string& name) {
    std::scoped_lock lock{mutex_};
    renderables_.erase(name);
}

void SceneNode::clear_absolute_observer() {
    std::scoped_lock lock{mutex_};
    if (absolute_observer_ != nullptr) {
        if (absolute_destruction_observer_ == nullptr) {
            verbose_abort("Internal error in clear_absolute_observer");
        }
        absolute_destruction_observer_ = nullptr;
        absolute_observer_ = nullptr;
    }
}

void SceneNode::clear() {
    clearing_observers.notify_destroyed();

    absolute_movable_ = nullptr;
    relative_movable_ = nullptr;
    node_modifier_ = nullptr;
    node_hiders_.clear();
    absolute_observer_ = nullptr;
    absolute_destruction_observer_ = nullptr;
    camera_ = nullptr;
    renderables_.clear();
    clear_map_recursively(children_, [this](const auto& child){
        if (child.mapped().is_registered) {
            // scene_ is non-null, checked in "add_child".
            scene_->unregister_node(child.key());
        }
    });
    clear_map_recursively(aggregate_children_, [this](const auto& child){
        if (child.mapped().is_registered) {
            // scene_ is non-null, checked in "add_child".
            scene_->unregister_node(child.key());
        }
    });
    aggregate_children_.clear();
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
    std::unique_ptr<SceneNode>&& node,
    ChildRegistrationState child_registration_state,
    ChildParentState child_parent_state)
{
    std::scoped_lock lock{mutex_};
    setup_child(name, *node, child_registration_state, child_parent_state);
    if (!children_.insert(std::make_pair(name, SceneNodeChild{
        .is_registered = (child_registration_state == ChildRegistrationState::REGISTERED),
        .scene_node = std::move(node)})).second)
    {
        THROW_OR_ABORT("Child node with name " + name + " already exists");
    }
}

SceneNode& SceneNode::get_child(const std::string& name) const {
    std::shared_lock lock{mutex_};
    auto it = children_.find(name);
    if (it == children_.end()) {
        THROW_OR_ABORT("Node does not have a child with name \"" + name + '"');
    }
    return *it->second.scene_node.get();
}

void SceneNode::remove_child(const std::string& name) {
    std::scoped_lock lock{mutex_};
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
        scene_->unregister_node(name);
    }
    children_.erase(it);
}

bool SceneNode::contains_child(const std::string& name) const {
    std::shared_lock lock{mutex_};
    return children_.find(name) != children_.end();
}

void SceneNode::add_aggregate_child(
    const std::string& name,
    std::unique_ptr<SceneNode>&& node,
    ChildRegistrationState child_registration_state,
    ChildParentState child_parent_state)
{
    std::scoped_lock lock{mutex_};
    setup_child(name, *node, child_registration_state, child_parent_state);
    if (!aggregate_children_.insert(std::make_pair(name, SceneNodeChild{
        .is_registered = (child_registration_state == ChildRegistrationState::REGISTERED),
        .scene_node = std::move(node)})).second)
    {
        THROW_OR_ABORT("Aggregate node with name " + name + " already exists");
    }
}

void SceneNode::add_instances_child(
    const std::string& name,
    std::unique_ptr<SceneNode>&& node,
    ChildRegistrationState child_registration_state,
    ChildParentState child_parent_state)
{
    std::scoped_lock lock{mutex_};
    setup_child(name, *node, child_registration_state, child_parent_state);
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
    std::scoped_lock lock{mutex_};
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
    for (const auto& [name, i] : instances_children_) {
        ostr << name << std::endl;
        i.small_instances.optimize_search_time(BvhDataRadiusType::ZERO, std::cerr);
        // i.small_instances.plot_svg<double>("/tmp/" + name + ".svg", 0, 1);
    }
}

bool SceneNode::has_camera() const {
    std::shared_lock lock{mutex_};
    return camera_ != nullptr;
}

void SceneNode::set_camera(std::unique_ptr<Camera>&& camera) {
    std::scoped_lock lock{mutex_};
    if (camera_ != nullptr) {
        THROW_OR_ABORT("Camera already set");
    }
    camera_ = std::move(camera);
}

Camera& SceneNode::get_camera() const {
    std::shared_lock lock{mutex_};
    if (camera_ == nullptr) {
        THROW_OR_ABORT("Node has no camera");
    }
    return *camera_.get();
}

void SceneNode::add_light(std::unique_ptr<Light>&& light) {
    std::scoped_lock lock{mutex_};
    lights_.push_back(std::move(light));
}

bool SceneNode::has_color_style(const std::string& name) const {
    std::shared_lock lock{mutex_};
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
    std::shared_lock lock{mutex_};
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
    std::scoped_lock lock{mutex_};
    if ((state_ != SceneNodeState::DETACHED) && !renderables_.empty()) {
        THROW_OR_ABORT(
            "Color style was set after renderables on a non-detached node. "
            "This leads to a race condition.");
    }
    color_styles_.push_back(std::move(color_style));
}

void SceneNode::set_animation_state(std::unique_ptr<AnimationState>&& animation_state) {
    std::scoped_lock lock{mutex_};
    if (!renderables_.empty()) {
        THROW_OR_ABORT("Animation state was set after renderables, this leads to a race condition");
    }
    if (animation_state_ != nullptr) {
        THROW_OR_ABORT("Scene node already has an animation state");
    }
    animation_state_ = std::move(animation_state);
}

void SceneNode::set_animation_state_updater(std::unique_ptr<AnimationStateUpdater>&& animation_state_updater) {
    std::scoped_lock lock{mutex_};
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
    SceneNodeResources* scene_node_resources,
    const AnimationState* animation_state)
{
    std::scoped_lock lock{mutex_};
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
            OffsetAndQuaternion<float, double> q0{position_, Quaternion<float>{rotation_matrix_}};
            OffsetAndQuaternion<float, double> q1{it->second.offset().casted<double>(), it->second.quaternion()};
            auto res_pose = q0.slerp(q1, 1.f - bone_.smoothness);
            res_pose.quaternion() = res_pose.quaternion().slerp(Quaternion<float>::identity(), 1.f - bone_.rotation_strength);
            set_relative_pose(
                res_pose.offset(),
                res_pose.quaternion().to_tait_bryan_angles(),
                scale());
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
        set_relative_pose(mr2.t(), matrix_2_tait_bryan_angles(mr2.R()), 1.f);
        v2 = relative_view_matrix() * v;
        absolute_movable_->set_absolute_model_matrix(v2.inverted_scaled());
    } else {
        if (absolute_movable_ != nullptr) {
            auto m = absolute_movable_->get_new_absolute_model_matrix();
            m = v * m;
            set_relative_pose(m.t(), matrix_2_tait_bryan_angles(m.R()), 1);
        }
        v2 = relative_view_matrix() * v;
        if (relative_movable_ != nullptr) {
            relative_movable_->set_absolute_model_matrix(v2.inverted_scaled());
            auto m = relative_movable_->get_new_relative_model_matrix();
            set_relative_pose(m.t(), matrix_2_tait_bryan_angles(m.R()), 1);
            v2 = relative_view_matrix() * v;
        }
    }
    if (absolute_observer_ != nullptr) {
        absolute_observer_->set_absolute_model_matrix(v2.inverted_scaled());
    }
    for (auto it = children_.begin(); it != children_.end(); ) {
        it->second.scene_node->move(v2, dt, scene_node_resources, estate);
        if (it->second.scene_node->to_be_deleted()) {
            remove_child((it++)->first);
        } else {
            ++it;
        }
    }
}

bool SceneNode::to_be_deleted() const {
    std::shared_lock lock{mutex_};
    return
        (animation_state_ != nullptr) &&
        animation_state_->delete_node_when_aperiodic_animation_finished &&
        animation_state_->aperiodic_animation_frame.ran_to_completion();
}

void SceneNode::set_bone(const SceneNodeBone& bone) {
    std::scoped_lock lock{mutex_};
    bone_ = bone;
}

void SceneNode::set_periodic_animation(const std::string& name) {
    std::scoped_lock lock{mutex_};
    periodic_animation_ = name;
}

void SceneNode::set_aperiodic_animation(const std::string& name) {
    std::scoped_lock lock{mutex_};
    aperiodic_animation_ = name;
}

bool SceneNode::requires_render_pass(ExternalRenderPassType render_pass) const {
    std::shared_lock lock{mutex_};
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
    const SceneNode& camera_node,
    const std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
    std::list<Blended>& blended,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    const ExternalRenderPass& external_render_pass,
    const AnimationState* animation_state,
    const std::list<const ColorStyle*>& color_styles,
    SceneNodeVisibility visibility) const
{
    std::shared_lock lock{mutex_};
    if (state_ == SceneNodeState::DETACHED) {
        THROW_OR_ABORT("Cannot render detached node");
    }
    for (const auto& nh : node_hiders_) {
        // Note that the NodeHider may depend on this function being called,
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
    FixedArray<double, 4, 4> mvp = dot2d(parent_mvp, relative_model_matrix().affine());
    auto m = parent_m * relative_model_matrix();
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
    std::shared_lock lock{mutex_};
    if (state_ != SceneNodeState::STATIC) {
        THROW_OR_ABORT("Cannot append sorted aggregates to queue for a non-static node");
    }
    // OpenGL matrices are transposed in memory,
    // https://stackoverflow.com/a/17718408/2292832.
    // "Note that post-multiplying with column-major matrices
    // produces the same result as pre-multiplying with
    // row-major matrices."
    FixedArray<double, 4, 4> mvp = dot2d(parent_mvp, relative_model_matrix().affine());
    auto m = parent_m * relative_model_matrix();
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
    std::shared_lock lock{mutex_};
    if (state_ != SceneNodeState::STATIC) {
        THROW_OR_ABORT("Cannot append large aggregates to queue for a non-static node");
    }
    TransformationMatrix<float, double, 3> m = parent_m * relative_model_matrix();
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
    std::shared_lock lock{mutex_};
    if (state_ != SceneNodeState::STATIC) {
        THROW_OR_ABORT("Cannot append small instances to queue for a non-static node");
    }
    TransformationMatrix<float, double, 3> rel = relative_model_matrix();
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
    std::shared_lock lock{mutex_};
    if (state_ != SceneNodeState::STATIC) {
        THROW_OR_ABORT("Cannot append large instances to queue for a non-static node");
    }
    TransformationMatrix<float, double, 3> rel = relative_model_matrix();
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
    std::shared_lock lock{mutex_};
    TransformationMatrix<float, double, 3> m = parent_m * relative_model_matrix();
    for (const auto& l : lights_) {
        lights.push_back(std::make_pair(m, l.get()));
    }
    for (const auto& [_, c] : children_) {
        c.scene_node->append_lights_to_queue(m, lights);
    }
}

const FixedArray<double, 3>& SceneNode::position() const {
    std::shared_lock lock{mutex_};
    return position_;
}

const FixedArray<float, 3>& SceneNode::rotation() const {
    std::shared_lock lock{mutex_};
    return rotation_;
}

float SceneNode::scale() const {
    std::shared_lock lock{mutex_};
    return scale_;
}

void SceneNode::set_position(const FixedArray<double, 3>& position) {
    std::scoped_lock lock{mutex_};
    if (state_ == SceneNodeState::STATIC) {
        THROW_OR_ABORT("Cannot set position for a static node");
    }
    position_ = position;
}

void SceneNode::set_rotation(const FixedArray<float, 3>& rotation) {
    std::scoped_lock lock{mutex_};
    if (state_ == SceneNodeState::STATIC) {
        THROW_OR_ABORT("Cannot set rotation for a static node");
    }
    rotation_ = rotation;
    rotation_matrix_ = tait_bryan_angles_2_matrix(rotation_);
}

void SceneNode::set_scale(float scale) {
    std::scoped_lock lock{mutex_};
    if (state_ == SceneNodeState::STATIC) {
        THROW_OR_ABORT("Cannot set scale for a static node");
    }
    scale_ = scale;
}

void SceneNode::set_relative_pose(
    const FixedArray<double, 3>& position,
    const FixedArray<float, 3>& rotation,
    float scale)
{
    set_position(position);
    set_rotation(rotation);
    set_scale(scale);
}

TransformationMatrix<float, double, 3> SceneNode::relative_model_matrix_unsafe() const {
    return TransformationMatrix{rotation_matrix_ * scale_, position_};
}

TransformationMatrix<float, double, 3> SceneNode::relative_model_matrix() const {
    std::shared_lock lock{mutex_};
    return relative_model_matrix_unsafe();
}

TransformationMatrix<float, double, 3> SceneNode::absolute_model_matrix() const {
    if (state_ != SceneNodeState::DETACHED) {
        scene_->delete_node_mutex().notify_reading();
    }
    auto result = relative_model_matrix_unsafe();
    if (parent_ != nullptr) {
        return parent_->absolute_model_matrix() * result;
    } else {
        return result;
    }
}

TransformationMatrix<float, double, 3> SceneNode::relative_view_matrix_unsafe() const {
    return TransformationMatrix<float, double, 3>::inverse(rotation_matrix_ / scale_, position_);
}

TransformationMatrix<float, double, 3> SceneNode::relative_view_matrix() const {
    std::shared_lock lock{mutex_};
    return relative_view_matrix_unsafe();
}

TransformationMatrix<float, double, 3> SceneNode::absolute_view_matrix() const {
    if (state_ != SceneNodeState::DETACHED) {
        scene_->delete_node_mutex().notify_reading();
    }
    auto result = relative_view_matrix_unsafe();
    if (parent_ != nullptr) {
        return result * parent_->absolute_view_matrix();
    } else {
        return result;
    }
}

void SceneNode::set_absolute_pose(
    const FixedArray<double, 3>& position,
    const FixedArray<float, 3>& rotation,
    float scale)
{
    std::scoped_lock lock{mutex_};
    if (parent_ == nullptr) {
        set_relative_pose(
            position,
            rotation,
            scale);
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
            rel_scale);
    }
}

std::optional<AxisAlignedBoundingBox<float, 3>> SceneNode::relative_aabb() const {
    std::shared_lock lock{mutex_};
    std::optional<AxisAlignedBoundingBox<float, 3>> result;
    if (!renderables_.empty()) {
        result = AxisAlignedBoundingBox<float, 3>();
    }
    for (const auto& [_, r] : renderables_) {
        result.value().extend(r->aabb());
    }
    for (const auto& [_, c] : children_) {
        auto cb = c.scene_node->relative_aabb();
        if (cb.has_value()) {
            auto m = c.scene_node->relative_model_matrix().casted<float, float>();
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
    std::shared_lock lock{mutex_};
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
    std::shared_lock lock{mutex_};
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
    std::scoped_lock lock{mutex_};
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
    for (const auto& [_, c] : children_) {
        c.scene_node->set_scene_and_state(scene, state);
    }
    for (const auto& [_, c] : aggregate_children_) {
        c.scene_node->set_scene_and_state(scene, state);
    }
    for (const auto& [_, c] : instances_children_) {
        c.scene_node->set_scene_and_state(scene, state);
    }
}

Scene& SceneNode::scene() {
    std::shared_lock lock{mutex_};
    if (scene_ == nullptr) {
        THROW_OR_ABORT("Scene not set");
    }
    return *scene_;
}

std::ostream& Mlib::operator << (std::ostream& ostr, const SceneNode& node) {
    node.print(ostr);
    return ostr;
}
