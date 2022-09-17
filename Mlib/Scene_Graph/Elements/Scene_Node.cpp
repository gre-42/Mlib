#include "Scene_Node.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Quaternion.hpp>
#include <Mlib/Recursive_Deletion.hpp>
#include <Mlib/Scene_Graph/Animation_State_Updater.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Animation_State.hpp>
#include <Mlib/Scene_Graph/Elements/Camera.hpp>
#include <Mlib/Scene_Graph/Elements/Color_Style.hpp>
#include <Mlib/Scene_Graph/Elements/Light.hpp>
#include <Mlib/Scene_Graph/Elements/Node_Hider.hpp>
#include <Mlib/Scene_Graph/Elements/Renderable.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Transformation/Absolute_Movable.hpp>
#include <Mlib/Scene_Graph/Transformation/Absolute_Observer.hpp>
#include <Mlib/Scene_Graph/Transformation/Node_Modifier.hpp>
#include <Mlib/Scene_Graph/Transformation/Relative_Movable.hpp>
#include <mutex>

using namespace Mlib;

SceneNode::SceneNode()
: destruction_observers{this},
  scene_{ nullptr },
  parent_{ nullptr },
  absolute_movable_{ nullptr },
  relative_movable_{ nullptr },
  node_hider_{ nullptr },
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
            std::cerr << "ERROR: Scene is null in static node" << std::endl;
            abort();
        }
        if (!scene_->shutting_down()) {
            std::cerr << "ERROR: Static node is being deleted but scene not shutting down" << std::endl;
            abort();
        }
    }
    destruction_observers.shutdown();
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
}

bool SceneNode::shutting_down() const {
    return destruction_observers.shutting_down();
}

bool SceneNode::has_parent() const {
    return (parent_ != nullptr);
}

SceneNode& SceneNode::parent() {
    std::shared_lock lock{mutex_};
    if (!has_parent()) {
        throw std::runtime_error("Node has no parent");
    }
    return *parent_;
}

void SceneNode::setup_child(const std::string& name, SceneNode& node, bool is_registered) {
    // Required in SceneNonde::~SceneNode
    if (is_registered && (scene_ == nullptr)) {
        throw std::runtime_error("Parent of registered node " + name + " does not have a scene");
    }
    if (name.empty()) {
        throw std::runtime_error("Child node has no name");
    }
    if (node.parent_ != nullptr) {
        throw std::runtime_error("Scene node \"" + name + "\" already has a parent");
    }
    node.parent_ = this;
    if ((scene_ != nullptr) != (state_ != SceneNodeState::DETACHED)) {
        throw std::runtime_error("Conflicting scene nullness and node state");
    }
    if (scene_ != nullptr) {
        node.set_scene_and_state(*scene_, state_);
    }
}

NodeModifier& SceneNode::get_node_modifier() const {
    std::shared_lock lock{mutex_};
    if (node_modifier_ == nullptr) {
        throw std::runtime_error("Node modifier not set");
    }
    return *node_modifier_;
}

void SceneNode::set_node_modifier(std::unique_ptr<NodeModifier>&& node_modifier)
{
    std::unique_lock lock{mutex_};
    if (node_modifier_ != nullptr) {
        throw std::runtime_error("Node modifier already set");
    }
    node_modifier_ = std::move(node_modifier);
}

void SceneNode::set_node_hider(NodeHider& node_hider) {
    std::unique_lock lock{mutex_};
    if (node_hider_ != nullptr) {
        throw std::runtime_error("Node hider already set");
    }
    node_hider_ = &node_hider;
}

void SceneNode::clear_node_hider() {
    std::unique_lock lock{mutex_};
    node_hider_ = nullptr;
}

AbsoluteMovable& SceneNode::get_absolute_movable() const {
    std::shared_lock lock{mutex_};
    if (absolute_movable_ == nullptr) {
        throw std::runtime_error("Absolute movable not set");
    }
    return *absolute_movable_;
}

void SceneNode::set_absolute_movable(const observer_ptr<AbsoluteMovable>& absolute_movable)
{
    std::unique_lock lock{mutex_};
    if (absolute_movable_ != nullptr) {
        throw std::runtime_error("Absolute movable already set");
    }
    absolute_movable_ = absolute_movable.get();
    absolute_movable_->set_absolute_model_matrix(absolute_model_matrix());
    if (absolute_movable.observer() != nullptr) {
        destruction_observers.add(absolute_movable.observer());
    }
}

RelativeMovable& SceneNode::get_relative_movable() const {
    std::shared_lock lock{mutex_};
    if (relative_movable_ == nullptr) {
        throw std::runtime_error("Relative movable not set");
    }
    return *relative_movable_;
}

void SceneNode::set_relative_movable(const observer_ptr<RelativeMovable>& relative_movable)
{
    std::unique_lock lock{mutex_};
    if (relative_movable_ != nullptr) {
        throw std::runtime_error("Relative movable already set");
    }
    relative_movable_ = relative_movable.get();
    relative_movable_->set_initial_relative_model_matrix(relative_model_matrix());
    relative_movable_->set_absolute_model_matrix(absolute_model_matrix());
    if (relative_movable.observer() != nullptr) {
        destruction_observers.add(relative_movable.observer());
    }
}

AbsoluteObserver& SceneNode::get_absolute_observer() const {
    std::shared_lock lock{mutex_};
    if (absolute_observer_ == nullptr) {
        throw std::runtime_error("Absolute observer not set");
    }
    return *absolute_observer_;
}

void SceneNode::set_absolute_observer(const observer_ptr<AbsoluteObserver>& absolute_observer)
{
    std::unique_lock lock{mutex_};
    if (absolute_observer_ != nullptr) {
        throw std::runtime_error("Absolute observer already set");
    }
    absolute_observer_ = absolute_observer.get();
    absolute_observer_->set_absolute_model_matrix(absolute_model_matrix());
    destruction_observers.add(absolute_observer.observer());

    absolute_destruction_observer_ = absolute_observer.observer();
}

void SceneNode::add_renderable(
    const std::string& name,
    const std::shared_ptr<const Renderable>& renderable)
{
    std::unique_lock lock{mutex_};
    if (name.empty()) {
        throw std::runtime_error("Renderable has no name");
    }
    if (!renderables_.insert(std::make_pair(name, renderable)).second) {
        throw std::runtime_error("Renderable with name " + name + " already exists");
    }
}

bool SceneNode::has_node_modifier() const {
    std::shared_lock lock{mutex_};
    return node_modifier_ != nullptr;
}

void SceneNode::clear_renderable_instance(const std::string& name) {
    std::unique_lock lock{mutex_};
    renderables_.erase(name);
}

void SceneNode::clear_absolute_observer_and_notify_destroyed() {
    std::unique_lock lock{mutex_};
    if (absolute_observer_ != nullptr) {
        destruction_observers.remove(absolute_destruction_observer_);
        absolute_destruction_observer_->notify_destroyed(this);
        absolute_destruction_observer_ = nullptr;
        absolute_observer_ = nullptr;
    }
}

void SceneNode::add_child(
    const std::string& name,
    std::unique_ptr<SceneNode>&& node,
    bool is_registered)
{
    std::unique_lock lock{mutex_};
    setup_child(name, *node, is_registered);
    if (!children_.insert(std::make_pair(name, SceneNodeChild{
        .is_registered = is_registered,
        .scene_node = std::move(node)})).second)
    {
        throw std::runtime_error("Child node with name " + name + " already exists");
    }
}

SceneNode& SceneNode::get_child(const std::string& name) const {
    std::shared_lock lock{mutex_};
    auto it = children_.find(name);
    if (it == children_.end()) {
        throw std::runtime_error("Node does not have a child with name \"" + name + '"');
    }
    return *it->second.scene_node.get();
}

void SceneNode::remove_child(const std::string& name) {
    std::unique_lock lock{mutex_};
    if (state_ == SceneNodeState::STATIC) {
        throw std::runtime_error("Cannot remove child \"" + name + "\" from static node");
    }
    auto it = children_.find(name);
    if (it == children_.end()) {
        throw std::runtime_error("Cannot not remove child with name \"" + name + "\" because it does not exist");
    }
    if (it->second.is_registered) {
        if (scene_ == nullptr) {
            throw std::runtime_error("Can not deregister child \"" + name + "\" because scene is not set");
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
    bool is_registered)
{
    std::unique_lock lock{mutex_};
    setup_child(name, *node, is_registered);
    if (!aggregate_children_.insert(std::make_pair(name, SceneNodeChild{
        .is_registered = is_registered,
        .scene_node = std::move(node)})).second)
    {
        throw std::runtime_error("Aggregate node with name " + name + " already exists");
    }
}

void SceneNode::add_instances_child(
    const std::string& name,
    std::unique_ptr<SceneNode>&& node,
    bool is_registered)
{
    std::unique_lock lock{mutex_};
    setup_child(name, *node, is_registered);
    if (!instances_children_.insert(std::make_pair(name, SceneNodeInstances{
        .is_registered = is_registered,
        .scene_node = std::move(node),
        .instances = std::move(std::list<PositionAndYAngle>())})).second)
    {
        throw std::runtime_error("Instances node with name " + name + " already exists");
    }
}

void SceneNode::add_instances_position(
    const std::string& name,
    const FixedArray<double, 3>& position,
    float yangle,
    uint32_t billboard_id)
{
    std::unique_lock lock{mutex_};
    auto it = instances_children_.find(name);
    if (it == instances_children_.end()) {
        throw std::runtime_error("Could not find instance node with name " + name);
    }
    it->second.instances.push_back(PositionAndYAngle{
        .position = position,
        .yangle = yangle,
        .billboard_id = billboard_id});
}

bool SceneNode::has_camera() const {
    std::shared_lock lock{mutex_};
    return camera_ != nullptr;
}

void SceneNode::set_camera(std::unique_ptr<Camera>&& camera) {
    std::unique_lock lock{mutex_};
    if (camera_ != nullptr) {
        throw std::runtime_error("Camera already set");
    }
    camera_ = std::move(camera);
}

Camera& SceneNode::get_camera() const {
    std::shared_lock lock{mutex_};
    if (camera_ == nullptr) {
        throw std::runtime_error("Node has no camera");
    }
    return *camera_.get();
}

void SceneNode::add_light(std::unique_ptr<Light>&& light) {
    std::unique_lock lock{mutex_};
    lights_.push_back(std::move(light));
}

bool SceneNode::has_color_style(const std::string& name) const {
    std::shared_lock lock{mutex_};
    bool style_found = false;
    for (const auto& s : color_styles_) {
        if (!re::regex_search(name, s->selector)) {
            continue;
        }
        if (style_found) {
            throw std::runtime_error("Node has multiple color styles matching \"" + name + '"');
        }
        style_found = true;
    }
    return style_found;
}

ColorStyle& SceneNode::color_style(const std::string& name) {
    std::shared_lock lock{mutex_};
    ColorStyle* result = nullptr;
    for (const auto& s : color_styles_) {
        if (!re::regex_search(name, s->selector)) {
            continue;
        }
        if (result != nullptr) {
            throw std::runtime_error("Node has multiple color styles matching \"" + name + '"');
        }
        result = s.get();
    }
    if (result == nullptr) {
        throw std::runtime_error("Node has no style matching \"" + name + '"');
    }
    return *result;
}

void SceneNode::add_color_style(std::unique_ptr<ColorStyle>&& color_style) {
    std::unique_lock lock{mutex_};
    if (!renderables_.empty()) {
        throw std::runtime_error("Color style was set after renderables, this leads to a race condition");
    }
    color_styles_.push_back(std::move(color_style));
}

void SceneNode::set_animation_state(std::unique_ptr<AnimationState>&& animation_state) {
    std::unique_lock lock{mutex_};
    if (!renderables_.empty()) {
        throw std::runtime_error("Animation state was set after renderables, this leads to a race condition");
    }
    if (animation_state_ != nullptr) {
        throw std::runtime_error("Scene node already has an animation state");
    }
    animation_state_ = std::move(animation_state);
}

void SceneNode::set_animation_state_updater(std::unique_ptr<AnimationStateUpdater>&& animation_state_updater) {
    std::unique_lock lock{mutex_};
    if (!renderables_.empty()) {
        throw std::runtime_error("Animation state updater was set after renderables, this leads to a race condition");
    }
    if (animation_state_updater_ != nullptr) {
        throw std::runtime_error("Scene node already has an animation state updater");
    }
    animation_state_updater_ = std::move(animation_state_updater);
}

void SceneNode::move(
    const TransformationMatrix<float, double, 3>& v,
    float dt,
    SceneNodeResources* scene_node_resources,
    const AnimationState* animation_state)
{
    std::unique_lock lock{mutex_};
    if (state_ == SceneNodeState::STATIC) {
        throw std::runtime_error("Cannot move static node");
    }
    if (node_modifier_ != nullptr) {
        node_modifier_->modify_node();
    }
    const AnimationState* estate = animation_state_ != nullptr
        ? animation_state_.get()
        : animation_state;
    if (!bone_.name.empty()) {
        if (estate == nullptr) {
            throw std::runtime_error("Bone name is not empty, but animation state is not set");
        }
        auto apply_scene_node_animation = [&](const AnimationFrame& animation_frame, const std::string& animation_name){
            if (animation_name.empty() || animation_name == "<no_animation>") {
                return;
            }
            if (scene_node_resources == nullptr) {
                throw std::runtime_error("Scene node animation without scene node resources");
            }
            if (std::isnan(animation_frame.time)) {
                throw std::runtime_error("Scene node animation loop time is NAN");
            }
            auto poses = scene_node_resources->get_absolute_poses(
                animation_name,
                animation_frame.time);
            auto it = poses.find(bone_.name);
            if (it == poses.end()) {
                throw std::runtime_error("Could not find bone with name \"node\" in animation \"" + animation_name + '"');
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
    std::unique_lock lock{mutex_};
    bone_ = bone;
}

void SceneNode::set_periodic_animation(const std::string& name) {
    std::unique_lock lock{mutex_};
    periodic_animation_ = name;
}

void SceneNode::set_aperiodic_animation(const std::string& name) {
    std::unique_lock lock{mutex_};
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
    const FixedArray<double, 4, 4>& vp,
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
        throw std::runtime_error("Cannot render detached node");
    }
    if ((node_hider_ != nullptr) &&
        any(external_render_pass.pass & ExternalRenderPassType::STANDARD_OR_IMPOSTOR_NODE) &&
        // Note that the NodeHider may depend on this function being called,
        // so there should not be any additional check above this line.
        node_hider_->node_shall_be_hidden(camera_node, external_render_pass))
    {
        visibility = SceneNodeVisibility::INVISIBLE;
    }
    // OpenGL matrices are transposed in memory,
    // https://stackoverflow.com/a/17718408/2292832.
    // "Note that post-multiplying with column-major matrices
    // produces the same result as pre-multiplying with
    // row-major matrices."
    FixedArray<double, 4, 4> mvp = dot2d(vp, relative_model_matrix().affine());
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
                if (re::regex_search(n, style->selector)) {
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
    const FixedArray<double, 4, 4>& vp,
    const TransformationMatrix<float, double, 3>& parent_m,
    const FixedArray<double, 3>& offset,
    std::list<std::pair<float, std::shared_ptr<ColoredVertexArray<float>>>>& aggregate_queue,
    const SceneGraphConfig& scene_graph_config,
    const ExternalRenderPass& external_render_pass) const
{
    std::shared_lock lock{mutex_};
    if (state_ != SceneNodeState::STATIC) {
        throw std::runtime_error("Cannot append sorted aggregates to queue for a non-static node");
    }
    // OpenGL matrices are transposed in memory,
    // https://stackoverflow.com/a/17718408/2292832.
    // "Note that post-multiplying with column-major matrices
    // produces the same result as pre-multiplying with
    // row-major matrices."
    FixedArray<double, 4, 4> mvp = dot2d(vp, relative_model_matrix().affine());
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
        throw std::runtime_error("Cannot append large aggregates to queue for a non-static node");
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
    const FixedArray<double, 4, 4>& vp,
    const TransformationMatrix<float, double, 3>& parent_m,
    const FixedArray<double, 3>& offset,
    const PositionAndYAngle& delta_pose,
    SmallInstancesQueues& instances_queues,
    const SceneGraphConfig& scene_graph_config) const
{
    std::shared_lock lock{mutex_};
    if (state_ != SceneNodeState::STATIC) {
        throw std::runtime_error("Cannot append small instances to queue for a non-static node");
    }
    TransformationMatrix<float, double, 3> rel = relative_model_matrix();
    rel.t() += delta_pose.position;
    if (delta_pose.yangle != 0) {
        rel.R() = dot2d(rel.R(), rodrigues2(FixedArray<float, 3>{0.f, 1.f, 0.f}, delta_pose.yangle));
    }
    FixedArray<double, 4, 4> mvp = dot2d(vp, rel.affine());
    TransformationMatrix<float, double, 3> m = parent_m * rel;
    for (const auto& [_, r] : renderables_) {
        r->append_sorted_instances_to_queue(mvp, m, offset, delta_pose.billboard_id, scene_graph_config, instances_queues);
    }
    for (const auto& [_, c] : children_) {
        c.scene_node->append_small_instances_to_queue(mvp, m, offset, PositionAndYAngle{fixed_zeros<double, 3>(), 0.f, UINT32_MAX}, instances_queues, scene_graph_config);
    }
    for (const auto& [_, i] : instances_children_) {
        // The transformation is swapped, meaning
        // y = P * V * M * INSTANCE * NODE * x.
        for (const auto& j : i.instances) {
            i.scene_node->append_small_instances_to_queue(mvp, m, offset, j, instances_queues, scene_graph_config);
        }
    }
}

void SceneNode::append_large_instances_to_queue(
    const FixedArray<double, 4, 4>& vp,
    const TransformationMatrix<float, double, 3>& parent_m,
    const FixedArray<double, 3>& offset,
    const PositionAndYAngle& delta_pose,
    LargeInstancesQueue& instances_queue,
    const SceneGraphConfig& scene_graph_config) const
{
    std::shared_lock lock{mutex_};
    if (state_ != SceneNodeState::STATIC) {
        throw std::runtime_error("Cannot append large instances to queue for a non-static node");
    }
    TransformationMatrix<float, double, 3> rel = relative_model_matrix();
    rel.t() += delta_pose.position;
    if (delta_pose.yangle != 0) {
        rel.R() = dot2d(rel.R(), rodrigues2(FixedArray<float, 3>{0.f, 1.f, 0.f}, delta_pose.yangle));
    }
    FixedArray<double, 4, 4> mvp = dot2d(vp, rel.affine());
    TransformationMatrix<float, double, 3> m = parent_m * rel;
    for (const auto& [_, r] : renderables_) {
        r->append_large_instances_to_queue(mvp, m, offset, delta_pose.billboard_id, scene_graph_config, instances_queue);
    }
    for (const auto& [_, c] : children_) {
        c.scene_node->append_large_instances_to_queue(mvp, m, offset, PositionAndYAngle{fixed_zeros<double, 3>(), 0.f, UINT32_MAX}, instances_queue, scene_graph_config);
    }
    for (const auto& [_, i] : instances_children_) {
        for (const auto& j : i.instances) {
            // The transformation is swapped, meaning
            // y = P * V * M * INSTANCE * NODE * x.
            i.scene_node->append_large_instances_to_queue(mvp, m, offset, j, instances_queue, scene_graph_config);
        }
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
    std::unique_lock lock{mutex_};
    if (state_ == SceneNodeState::STATIC) {
        throw std::runtime_error("Cannot set position for a static node");
    }
    position_ = position;
}

void SceneNode::set_rotation(const FixedArray<float, 3>& rotation) {
    std::unique_lock lock{mutex_};
    if (state_ == SceneNodeState::STATIC) {
        throw std::runtime_error("Cannot set rotation for a static node");
    }
    rotation_ = rotation;
    rotation_matrix_ = tait_bryan_angles_2_matrix(rotation_);
}

void SceneNode::set_scale(float scale) {
    std::unique_lock lock{mutex_};
    if (state_ == SceneNodeState::STATIC) {
        throw std::runtime_error("Cannot set scale for a static node");
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

TransformationMatrix<float, double, 3> SceneNode::relative_model_matrix() const {
    std::shared_lock lock{mutex_};
    return TransformationMatrix{rotation_matrix_ * scale_, position_};
}

TransformationMatrix<float, double, 3> SceneNode::absolute_model_matrix() const {
    std::shared_lock lock{mutex_};
    auto result = relative_model_matrix();
    if (parent_ != nullptr) {
        return parent_->absolute_model_matrix() * result;
    } else {
        return result;
    }
}

TransformationMatrix<float, double, 3> SceneNode::relative_view_matrix() const {
    std::shared_lock lock{mutex_};
    return TransformationMatrix<float, double, 3>::inverse(rotation_matrix_ / scale_, position_);
}

TransformationMatrix<float, double, 3> SceneNode::absolute_view_matrix() const {
    std::shared_lock lock{mutex_};
    auto result = relative_view_matrix();
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
    std::unique_lock lock{mutex_};
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
            ostr << " " << ind2 << " " << n << " n=" << c.instances.size() << '\n';
            c.scene_node->print(ostr, recursion_depth + 1);
        }
    }
    ostr << " " << ind0 << " End\n";
}

void SceneNode::set_scene_and_state(Scene& scene, SceneNodeState state) {
    std::unique_lock lock{mutex_};
    if (scene_ != nullptr) {
        throw std::runtime_error("Scene node already has a scene");
    }
    scene_ = &scene;
    if (state_ != SceneNodeState::DETACHED) {
        throw std::runtime_error("Node state already set");
    }
    if (state == SceneNodeState::DETACHED) {
        throw std::runtime_error("Cannot set node state to \"detached\"");
    }
    if ((state == SceneNodeState::STATIC) && (scene_ == nullptr)) {
        throw std::runtime_error("Scene is null in static node");
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
        throw std::runtime_error("Scene not set");
    }
    return *scene_;
}

std::ostream& Mlib::operator << (std::ostream& ostr, const SceneNode& node) {
    node.print(ostr);
    return ostr;
}
