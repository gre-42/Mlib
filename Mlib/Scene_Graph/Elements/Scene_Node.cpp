#include "Scene_Node.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Quaternion.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Recursive_Deletion.hpp>
#include <Mlib/Scene_Graph/Animation_State_Updater.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Animation_State.hpp>
#include <Mlib/Scene_Graph/Elements/Color_Style.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

SceneNode::SceneNode(Scene* scene)
: scene_{ scene },
  parent_{ nullptr },
  absolute_movable_{ nullptr },
  relative_movable_{ nullptr },
  absolute_observer_{ nullptr },
  absolute_destruction_observer_{ nullptr },
  position_{ 0.f, 0.f, 0.f },
  rotation_{ 0.f, 0.f, 0.f },
  scale_{ 1.f },
  rotation_matrix_{ fixed_identity_array<float, 3>() },
  shutting_down_{ false }
{}

SceneNode::~SceneNode() {
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wterminate"
    if (shutting_down_) {
        throw std::runtime_error("Scene node already shutting down");
    }
    #pragma GCC diagnostic pop
    shutting_down_ = true;
    clear_set_recursively(destruction_observers_, [this](const auto& obs){
        obs->notify_destroyed(this);
    });
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
    return shutting_down_;
}

SceneNode* SceneNode::parent() {
    if (parent_ == nullptr) {
        throw std::runtime_error("Node has no parent");
    }
    return parent_;
}

void SceneNode::set_parent(SceneNode& parent) {
    if (parent_ != nullptr) {
        throw std::runtime_error("Scene node already has a parent");
    }
    parent_ = &parent;
}

NodeModifier* SceneNode::get_node_modifier() const {
    if (node_modifier_ == nullptr) {
        throw std::runtime_error("Node modifier not set");
    }
    return node_modifier_.get();
}

void SceneNode::set_node_modifier(std::unique_ptr<NodeModifier>&& node_modifier)
{
    if (node_modifier_ != nullptr) {
        throw std::runtime_error("Node modifier already set");
    }
    node_modifier_ = std::move(node_modifier);
}

AbsoluteMovable* SceneNode::get_absolute_movable() const {
    if (absolute_movable_ == nullptr) {
        throw std::runtime_error("Absolute movable not set");
    }
    return absolute_movable_;
}

void SceneNode::set_absolute_movable(const observer_ptr<AbsoluteMovable>& absolute_movable)
{
    if (absolute_movable_ != nullptr) {
        throw std::runtime_error("Absolute movable already set");
    }
    absolute_movable_ = absolute_movable.get();
    absolute_movable_->set_absolute_model_matrix(absolute_model_matrix());
    add_destruction_observer(absolute_movable.observer());
}

RelativeMovable* SceneNode::get_relative_movable() const {
    if (relative_movable_ == nullptr) {
        throw std::runtime_error("Relative movable not set");
    }
    return relative_movable_;
}

void SceneNode::set_relative_movable(const observer_ptr<RelativeMovable>& relative_movable)
{
    if (relative_movable_ != nullptr) {
        throw std::runtime_error("Relative movable already set");
    }
    relative_movable_ = relative_movable.get();
    relative_movable_->set_initial_relative_model_matrix(relative_model_matrix());
    relative_movable_->set_absolute_model_matrix(absolute_model_matrix());
    add_destruction_observer(relative_movable.observer());
}

AbsoluteObserver* SceneNode::get_absolute_observer() const {
    if (absolute_observer_ == nullptr) {
        throw std::runtime_error("Absolute observer not set");
    }
    return absolute_observer_;
}

void SceneNode::set_absolute_observer(const observer_ptr<AbsoluteObserver>& absolute_observer)
{
    if (absolute_observer_ != nullptr) {
        throw std::runtime_error("Absolute observer already set");
    }
    absolute_observer_ = absolute_observer.get();
    absolute_observer_->set_absolute_model_matrix(absolute_model_matrix());
    add_destruction_observer(absolute_observer.observer());

    absolute_destruction_observer_ = absolute_observer.observer();
}

void SceneNode::add_destruction_observer(DestructionObserver* destruction_observer, bool ignore_exists) {
    auto r = destruction_observers_.insert(destruction_observer);
    if (!ignore_exists && !r.second) {
        throw std::runtime_error("Destruction observer already registered");
    }
}

void SceneNode::remove_destruction_observer(
    DestructionObserver* destruction_observer,
    bool ignore_not_exists) {
    if (!shutting_down()) {
        size_t nerased = destruction_observers_.erase(destruction_observer);
        if (!ignore_not_exists && (nerased != 1)) {
            throw std::runtime_error("Could not find destruction observer to be erased");
        }
    }
}

void SceneNode::add_renderable(
    const std::string& name,
    const std::shared_ptr<const Renderable>& renderable)
{
    if (name.empty()) {
        throw std::runtime_error("Renderable has no name");
    }
    if (!renderables_.insert(std::make_pair(name, renderable)).second) {
        throw std::runtime_error("Renderable with name " + name + " already exists");
    }
}

void SceneNode::clear_renderable_instance(const std::string& name) {
    renderables_.erase(name);
}

void SceneNode::clear_absolute_observer() {
    if (absolute_observer_ != nullptr) {
        if (destruction_observers_.erase(absolute_destruction_observer_) != 1) {
            throw std::runtime_error("Could not find absolute destruction observer for deletion");
        }
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
    // Required in SceneNonde::~SceneNode
    if (is_registered && (scene_ == nullptr)) {
        throw std::runtime_error("Parent of registered node " + name + " does not have a scene");
    }
    if (name.empty()) {
        throw std::runtime_error("Child node has no name");
    }
    SceneNode* n = node.get();
    if (!children_.insert(std::make_pair(name, SceneNodeChild{
        .is_registered = is_registered,
        .scene_node = std::move(node)})).second)
    {
        throw std::runtime_error("Child node with name " + name + " already exists");
    }
    n->set_parent(*this);
}

SceneNode& SceneNode::get_child(const std::string& name) const {
    auto it = children_.find(name);
    if (it == children_.end()) {
        throw std::runtime_error("Node does not have a child with name \"" + name + '"');
    }
    return *it->second.scene_node.get();
}

void SceneNode::remove_child(const std::string& name) {
    if (children_.erase(name) != 1) {
        throw std::runtime_error("Could not erase child with name \"" + name + '"');
    }
}

bool SceneNode::contains_child(const std::string& name) const {
    return children_.find(name) != children_.end();
}

void SceneNode::add_aggregate_child(
    const std::string& name,
    std::unique_ptr<SceneNode>&& node,
    bool is_registered)
{
    // Required in SceneNonde::~SceneNode
    if (is_registered && (scene_ == nullptr)) {
        throw std::runtime_error("Parent of registered node " + name + " does not have a scene");
    }
    if (name.empty()) {
        throw std::runtime_error("Child node has no name");
    }
    SceneNode* n = node.get();
    if (!aggregate_children_.insert(std::make_pair(name, SceneNodeChild{
        .is_registered = is_registered,
        .scene_node = std::move(node)})).second)
    {
        throw std::runtime_error("Aggregate node with name " + name + " already exists");
    }
    n->set_parent(*this);
}

void SceneNode::add_instances_child(
    const std::string& name,
    std::unique_ptr<SceneNode>&& node,
    bool is_registered)
{
    // Required in SceneNonde::~SceneNode
    if (is_registered && (scene_ == nullptr)) {
        throw std::runtime_error("Parent of registered node " + name + " does not have a scene");
    }
    if (name.empty()) {
        throw std::runtime_error("Child node has no name");
    }
    SceneNode* n = node.get();
    if (!instances_children_.insert(std::make_pair(name, SceneNodeInstances{
        .is_registered = is_registered,
        .scene_node = std::move(node),
        .instances = std::move(std::list<PositionAndYAngle>())})).second)
    {
        throw std::runtime_error("Instances node with name " + name + " already exists");
    }
    n->set_parent(*this);
}

void SceneNode::add_instances_position(
    const std::string& name,
    const FixedArray<float, 3>& position,
    float yangle,
    uint32_t billboard_id)
{
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
    return camera_ != nullptr;
}

void SceneNode::set_camera(std::unique_ptr<Camera>&& camera) {
    if (camera_ != nullptr) {
        throw std::runtime_error("Camera already set");
    }
    camera_ = std::move(camera);
}

Camera& SceneNode::get_camera() const {
    if (camera_ == nullptr) {
        throw std::runtime_error("Node has no camera");
    }
    return *camera_.get();
}

void SceneNode::add_light(std::unique_ptr<Light>&& light) {
    lights_.push_back(std::move(light));
}

bool SceneNode::has_color_style(const std::string& name) const {
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
    if (!renderables_.empty()) {
        throw std::runtime_error("Color style was set after renderables, this leads to a race condition");
    }
    color_styles_.push_back(std::move(color_style));
}

void SceneNode::set_animation_state(std::unique_ptr<AnimationState>&& animation_state) {
    if (!renderables_.empty()) {
        throw std::runtime_error("Animation state was set after renderables, this leads to a race condition");
    }
    if (animation_state_ != nullptr) {
        throw std::runtime_error("Scene node already has an animation state");
    }
    animation_state_ = std::move(animation_state);
}

void SceneNode::set_animation_state_updater(std::unique_ptr<AnimationStateUpdater>&& animation_state_updater) {
    if (!renderables_.empty()) {
        throw std::runtime_error("Animation state updater was set after renderables, this leads to a race condition");
    }
    if (animation_state_updater_ != nullptr) {
        throw std::runtime_error("Scene node already has an animation state updater");
    }
    animation_state_updater_ = std::move(animation_state_updater);
}

void SceneNode::move(
    const TransformationMatrix<float, 3>& v,
    float dt,
    SceneNodeResources* scene_node_resources,
    const AnimationState* animation_state)
{
    if (node_modifier_ != nullptr) {
        node_modifier_->modify_node();
    }
    const AnimationState* estate = animation_state_ != nullptr
        ? animation_state_.get()
        : animation_state;
    if (animation_state != nullptr) {
        auto apply_scene_node_animation = [&](const AnimationFrame& animation_frame, const std::string& animation_name){
            if (animation_name.empty()) {
                return;
            }
            if (scene_node_resources == nullptr) {
                throw std::runtime_error("Scene node animation without scene node resources");
            }
            if (std::isnan(animation_frame.time)) {
                throw std::runtime_error("Scene node animation loop time is NAN");
            }
            auto poses = scene_node_resources->get_poses(
                animation_name,
                animation_frame.time);
            auto it = poses.find("node");
            if (it == poses.end()) {
                throw std::runtime_error("Could not find bone with name \"node\" in animation \"" + animation_name + '"');
            }
            set_relative_pose(
                it->second.offset(),
                it->second.quaternion().to_tait_bryan_angles(),
                scale());
        };
        if (estate->aperiodic_animation_frame.active()) {
            apply_scene_node_animation(estate->aperiodic_animation_frame.frame, aperiodic_animation_);
        } else {
            apply_scene_node_animation(estate->periodic_skelletal_animation_frame.frame, periodic_animation_);
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
    TransformationMatrix<float, 3> v2;
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
    for (const auto& [n, c] : children_) {
        c.scene_node->move(v2, dt, scene_node_resources, estate);
    }
}

bool SceneNode::to_be_deleted() const {
    return
        (animation_state_ != nullptr) &&
        animation_state_->delete_node_when_aperiodic_animation_finished &&
        animation_state_->aperiodic_animation_frame.ran_to_completion();
}

void SceneNode::set_periodic_animation(const std::string& name) {
    periodic_animation_ = name;
}

void SceneNode::set_aperiodic_animation(const std::string& name) {
    aperiodic_animation_ = name;
}

bool SceneNode::requires_render_pass(ExternalRenderPassType render_pass) const {
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
    const FixedArray<float, 4, 4>& vp,
    const TransformationMatrix<float, 3>& parent_m,
    const TransformationMatrix<float, 3>& iv,
    const SceneNode& camera_node,
    const std::list<std::pair<TransformationMatrix<float, 3>, Light*>>& lights,
    std::list<Blended>& blended,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    const ExternalRenderPass& external_render_pass,
    const AnimationState* animation_state,
    const std::list<const ColorStyle*>& color_styles) const
{
    // OpenGL matrices are transposed in memory,
    // https://stackoverflow.com/a/17718408/2292832.
    // "Note that post-multiplying with column-major matrices
    // produces the same result as pre-multiplying with
    // row-major matrices."
    FixedArray<float, 4, 4> mvp = dot2d(vp, relative_model_matrix().affine());
    auto m = parent_m * relative_model_matrix();
    const AnimationState* estate = animation_state_ != nullptr
        ? animation_state_.get()
        : animation_state;
    std::list<const ColorStyle*> ecolor_styles = color_styles;
    for (const auto& s : color_styles_) {
        ecolor_styles.push_back(s.get());
    }
    for (const auto& [n, r] : renderables_) {
        r->notify_rendering(*this, camera_node);
        ColorStyle r_style;
        for (const auto& style : ecolor_styles) {
            if (!re::regex_search(n, style->selector)) {
                continue;
            }
            if (!all(style->ambience == -1.f)) {
                r_style.ambience = style->ambience;
            }
            if (!all(style->diffusivity == -1.f)) {
                r_style.diffusivity = style->diffusivity;
            }
            if (!all(style->specularity == -1.f)) {
                r_style.specularity = style->specularity;
            }
            for (const auto& [key, value] : style->reflection_maps) {
                r_style.reflection_maps[key] = value;
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
        if (r->requires_blending_pass())
        {
            blended.push_back(Blended{
                .z_order = r->continuous_blending_z_order(),
                .mvp = mvp,
                .m = m,
                .renderable = r.get(),
                .color_style = std::move(r_style)});
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
            ecolor_styles);
    }
}

void SceneNode::append_sorted_aggregates_to_queue(
    const FixedArray<float, 4, 4>& vp,
    const TransformationMatrix<float, 3>& parent_m,
    std::list<std::pair<float, std::shared_ptr<ColoredVertexArray>>>& aggregate_queue,
    const SceneGraphConfig& scene_graph_config,
    const ExternalRenderPass& external_render_pass) const
{
    // OpenGL matrices are transposed in memory,
    // https://stackoverflow.com/a/17718408/2292832.
    // "Note that post-multiplying with column-major matrices
    // produces the same result as pre-multiplying with
    // row-major matrices."
    FixedArray<float, 4, 4> mvp = dot2d(vp, relative_model_matrix().affine());
    auto m = parent_m * relative_model_matrix();
    for (const auto& [_, r] : renderables_) {
        r->append_sorted_aggregates_to_queue(mvp, m, scene_graph_config, external_render_pass, aggregate_queue);
    }
    for (const auto& [_, c] : children_) {
        c.scene_node->append_sorted_aggregates_to_queue(mvp, m, aggregate_queue, scene_graph_config, external_render_pass);
    }
    for (const auto& [_, a] : aggregate_children_) {
        a.scene_node->append_sorted_aggregates_to_queue(mvp, m, aggregate_queue, scene_graph_config, external_render_pass);
    }
}

void SceneNode::append_large_aggregates_to_queue(
    const TransformationMatrix<float, 3>& parent_m,
    std::list<std::shared_ptr<ColoredVertexArray>>& aggregate_queue,
    const SceneGraphConfig& scene_graph_config) const
{
    TransformationMatrix<float, 3> m = parent_m * relative_model_matrix();
    for (const auto& [_, r] : renderables_) {
        r->append_large_aggregates_to_queue(m, scene_graph_config, aggregate_queue);
    }
    for (const auto& [_, c] : children_) {
        c.scene_node->append_large_aggregates_to_queue(m, aggregate_queue, scene_graph_config);
    }
    for (const auto& [_, a] : aggregate_children_) {
        a.scene_node->append_large_aggregates_to_queue(m, aggregate_queue, scene_graph_config);
    }
}

void SceneNode::append_small_instances_to_queue(
    const FixedArray<float, 4, 4>& vp,
    const TransformationMatrix<float, 3>& parent_m,
    const PositionAndYAngle& delta_pose,
    std::list<std::pair<float, TransformedColoredVertexArray>>& instances_queue,
    const SceneGraphConfig& scene_graph_config,
    const ExternalRenderPass& external_render_pass) const
{
    TransformationMatrix<float, 3> rel = relative_model_matrix();
    rel.t() += delta_pose.position;
    if (delta_pose.yangle != 0) {
        rel.R() = dot2d(rel.R(), rodrigues2(FixedArray<float, 3>{0.f, 1.f, 0.f}, delta_pose.yangle));
    }
    FixedArray<float, 4, 4> mvp = dot2d(vp, rel.affine());
    TransformationMatrix<float, 3> m = parent_m * rel;
    for (const auto& [_, r] : renderables_) {
        r->append_sorted_instances_to_queue(mvp, m, delta_pose.billboard_id, scene_graph_config, external_render_pass, instances_queue);
    }
    for (const auto& [_, c] : children_) {
        c.scene_node->append_small_instances_to_queue(mvp, m, PositionAndYAngle{fixed_zeros<float, 3>(), 0.f, UINT32_MAX}, instances_queue, scene_graph_config, external_render_pass);
    }
    for (const auto& [_, i] : instances_children_) {
        // The transformation is swapped, meaning
        // y = P * V * M * INSTANCE * NODE * x.
        for (const auto& j : i.instances) {
            i.scene_node->append_small_instances_to_queue(mvp, m, j, instances_queue, scene_graph_config, external_render_pass);
        }
    }
}

void SceneNode::append_large_instances_to_queue(
    const TransformationMatrix<float, 3>& parent_m,
    const PositionAndYAngle& delta_pose,
    std::list<TransformedColoredVertexArray>& instances_queue,
    const SceneGraphConfig& scene_graph_config) const
{
    TransformationMatrix<float, 3> rel = relative_model_matrix();
    rel.t() += delta_pose.position;
    if (delta_pose.yangle != 0) {
        rel.R() = dot2d(rel.R(), rodrigues2(FixedArray<float, 3>{0.f, 0.f, 1.f}, delta_pose.yangle));
    }
    TransformationMatrix<float, 3> m = parent_m * rel;
    for (const auto& [_, r] : renderables_) {
        r->append_large_instances_to_queue(m, delta_pose.billboard_id, scene_graph_config, instances_queue);
    }
    for (const auto& [_, c] : children_) {
        c.scene_node->append_large_instances_to_queue(m, PositionAndYAngle{fixed_zeros<float, 3>(), 0.f, UINT32_MAX}, instances_queue, scene_graph_config);
    }
    for (const auto& [_, i] : instances_children_) {
        for (const auto& j : i.instances) {
            // The transformation is swapped, meaning
            // y = P * V * M * INSTANCE * NODE * x.
            i.scene_node->append_large_instances_to_queue(m, j, instances_queue, scene_graph_config);
        }
    }
}

void SceneNode::append_lights_to_queue(
    const TransformationMatrix<float, 3>& parent_m,
    std::list<std::pair<TransformationMatrix<float, 3>, Light*>>& lights) const
{
    TransformationMatrix<float, 3> m = parent_m * relative_model_matrix();
    for (const auto& l : lights_) {
        lights.push_back(std::make_pair(m, l.get()));
    }
    for (const auto& [_, c] : children_) {
        c.scene_node->append_lights_to_queue(m, lights);
    }
}

const FixedArray<float, 3>& SceneNode::position() const {
    return position_;
}

const FixedArray<float, 3>& SceneNode::rotation() const {
    return rotation_;
}

float SceneNode::scale() const {
    return scale_;
}

void SceneNode::set_position(const FixedArray<float, 3>& position) {
    position_ = position;
}

void SceneNode::set_rotation(const FixedArray<float, 3>& rotation) {
    rotation_ = rotation;
    rotation_matrix_ = tait_bryan_angles_2_matrix(rotation_);
}

void SceneNode::set_scale(float scale) {
    scale_ = scale;
}

void SceneNode::set_relative_pose(
    const FixedArray<float, 3>& position,
    const FixedArray<float, 3>& rotation,
    float scale)
{
    set_position(position);
    set_rotation(rotation);
    set_scale(scale);
}

TransformationMatrix<float, 3> SceneNode::relative_model_matrix() const {
    return TransformationMatrix{rotation_matrix_ * scale_, position_};
}

TransformationMatrix<float, 3> SceneNode::absolute_model_matrix() const {
    TransformationMatrix<float, 3> result = relative_model_matrix();
    if (parent_ != nullptr) {
        return parent_->absolute_model_matrix() * result;
    } else {
        return result;
    }
}

TransformationMatrix<float, 3> SceneNode::relative_view_matrix() const {
    return TransformationMatrix<float, 3>::inverse(rotation_matrix_ / scale_, position_);
}

TransformationMatrix<float, 3> SceneNode::absolute_view_matrix() const {
    TransformationMatrix<float, 3> result = relative_view_matrix();
    if (parent_ != nullptr) {
        return result * parent_->absolute_view_matrix();
    } else {
        return result;
    }
}

void SceneNode::set_absolute_pose(
    const FixedArray<float, 3>& position,
    const FixedArray<float, 3>& rotation,
    float scale)
{
    if (parent_ == nullptr) {
        set_relative_pose(
            position,
            rotation,
            scale);
    } else {
        auto p_v = parent_->absolute_view_matrix();
        auto m = TransformationMatrix<float, 3>{
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

void SceneNode::print(std::ostream& ostr, size_t recursion_depth) const {
    std::string ind0(3 * recursion_depth, '-');
    std::string ind1(3 * recursion_depth + 1, '-');
    std::string ind2(3 * recursion_depth + 2, '-');
    ostr << " " << ind0 << " Node\n";
    ostr << " " << ind1 << " Position " << position() << '\n';
    ostr << " " << ind1 << " Rotation " << rotation() << '\n';
    if (!renderables_.empty()) {
        ostr << " " << ind1 << " sResource (" << renderables_.size() << ")\n";
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

std::ostream& Mlib::operator << (std::ostream& ostr, const SceneNode& node) {
    node.print(ostr);
    return ostr;
}
