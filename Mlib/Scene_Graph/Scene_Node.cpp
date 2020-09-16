#include "Scene_Node.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>

using namespace Mlib;

SceneNode::SceneNode(Scene* scene)
: scene_{scene},
  parent_{nullptr},
  absolute_movable_{nullptr},
  relative_movable_{nullptr},
  absolute_observer_{nullptr},
  position_{0, 0, 0},
  rotation_{0, 0, 0},
  scale_{1},
  rotation_matrix_invalidated_{true}
{}

SceneNode::~SceneNode() {
    for(auto& o : destruction_observers_) {
        o->notify_destroyed(this);
    }
    for(auto& c : children_) {
        if (c.second.first) {
            // scene_ is non-null, checked in "add_child".
            scene_->unregister_node(c.first);
        }
    }
    for(auto& c : aggregate_children_) {
        if (c.second.first) {
            // scene_ is non-null, checked in "add_child".
            scene_->unregister_node(c.first);
        }
    }
}

void SceneNode::set_parent(SceneNode* parent) {
    parent_ = parent;
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
}

void SceneNode::add_destruction_observer(DestructionObserver* destruction_observer) {
    auto r = destruction_observers_.insert(destruction_observer);
    if (!r.second) {
        throw std::runtime_error("Destruction observer already registered");
    }
}

void SceneNode::remove_destruction_observer(DestructionObserver* destruction_observer) {
    size_t nerased = destruction_observers_.erase(destruction_observer);
    if (nerased != 1) {
        throw std::runtime_error("Could not find destruction observer to be erased");
    }
}

void SceneNode::add_renderable(
    const std::string& name,
    const std::shared_ptr<Renderable>& renderable)
{
    if (renderables_.find(name) != renderables_.end()) {
        throw std::runtime_error("Renderable with name " + name + " already exists");
    }
    renderables_.insert(std::make_pair(name, renderable));
}

void SceneNode::add_child(
    const std::string& name,
    SceneNode* node,
    bool is_registered)
{
    if (children_.find(name) != children_.end()) {
        throw std::runtime_error("Child node with name " + name + " already exists");
    }
    // Required in SceneNonde::~SceneNode
    if (is_registered && (scene_ == nullptr)) {
        throw std::runtime_error("Parent of registered node " + name + " does not have a scene");
    }
    children_.insert(std::make_pair(name, std::make_pair(is_registered, node)));
}

void SceneNode::add_aggregate_child(
    const std::string& name,
    SceneNode* node,
    bool is_registered)
{
    if (aggregate_children_.find(name) != aggregate_children_.end()) {
        throw std::runtime_error("Aggregate node with name " + name + " already exists");
    }
    // Required in SceneNonde::~SceneNode
    if (is_registered && (scene_ == nullptr)) {
        throw std::runtime_error("Parent of registered node " + name + " does not have a scene");
    }
    aggregate_children_.insert(std::make_pair(name, std::make_pair(is_registered, node)));
}

void SceneNode::add_instances_child(
    const std::string& name,
    SceneNode* node,
    bool is_registered)
{
    if (instances_children_.find(name) != instances_children_.end()) {
        throw std::runtime_error("Aggregate node with name " + name + " already exists");
    }
    // Required in SceneNonde::~SceneNode
    if (is_registered && (scene_ == nullptr)) {
        throw std::runtime_error("Parent of registered node " + name + " does not have a scene");
    }
    instances_children_.insert(std::make_pair(name, std::make_pair(is_registered, node)));
}

void SceneNode::set_camera(const std::shared_ptr<Camera>& camera) {
    if (camera_ != nullptr) {
        throw std::runtime_error("Camera already set");
    }
    camera_ = camera;
}

std::shared_ptr<Camera> SceneNode::get_camera() const {
    if (camera_ == nullptr) {
        throw std::runtime_error("Node has no camera");
    }
    return camera_;
}

void SceneNode::add_light(Light* light) {
    lights_.push_back(std::unique_ptr<Light>{light});
}

void SceneNode::move(const FixedArray<float, 4, 4>& v) {
    FixedArray<float, 4, 4> v2;
    if ((absolute_movable_ != nullptr) && (relative_movable_ != nullptr)) {
        FixedArray<float, 4, 4> ma = absolute_movable_->get_new_absolute_model_matrix();
        FixedArray<float, 4, 4> mr = dot2d(v, ma);
        relative_movable_->set_updated_relative_model_matrix(mr);
        relative_movable_->set_absolute_model_matrix(ma);
        FixedArray<float, 4, 4> mr2 = relative_movable_->get_new_relative_model_matrix();
        set_relative_pose(t3_from_4x4(mr2), matrix_2_tait_bryan_angles(R3_from_4x4(mr2)), 1);
        v2 = dot2d(relative_view_matrix(), v);
        absolute_movable_->set_absolute_model_matrix(inverted_scaled_se3(v2));
    } else {
        if (absolute_movable_ != nullptr) {
            FixedArray<float, 4, 4> m = absolute_movable_->get_new_absolute_model_matrix();
            m = dot2d(v, m);
            set_relative_pose(t3_from_4x4(m), matrix_2_tait_bryan_angles(R3_from_4x4(m)), 1);
        }
        v2 = dot2d(relative_view_matrix(), v);
        if (relative_movable_ != nullptr) {
            relative_movable_->set_absolute_model_matrix(inverted_scaled_se3(v2));
            FixedArray<float, 4, 4> m = relative_movable_->get_new_relative_model_matrix();
            set_relative_pose(t3_from_4x4(m), matrix_2_tait_bryan_angles(R3_from_4x4(m)), 1);
            v2 = dot2d(relative_view_matrix(), v);
        }
    }
    if (absolute_observer_ != nullptr) {
        absolute_observer_->set_absolute_model_matrix(inverted_scaled_se3(v2));
    }
    for(const auto& n : children_) {
        n.second.second->move(v2);
    }
}

bool SceneNode::requires_render_pass() const {
    for(const auto& r : renderables_) {
        if (r.second->requires_render_pass()) {
            return true;
        }
    }
    for(const auto& n : children_) {
        if (n.second.second->requires_render_pass()) {
            return true;
        }
    }
    return false;
}

void SceneNode::render(
    const FixedArray<float, 4, 4>& vp,
    const FixedArray<float, 4, 4>& parent_m,
    const FixedArray<float, 4, 4>& iv,
    const std::list<std::pair<FixedArray<float, 4, 4>, Light*>>& lights,
    std::list<Blended>& blended,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    ExternalRenderPass external_render_pass) const
{
    // OpenGL matrices are transposed in memory,
    // https://stackoverflow.com/a/17718408/2292832.
    // "Note that post-multiplying with column-major matrices
    // produces the same result as pre-multiplying with
    // row-major matrices."
    FixedArray<float, 4, 4> mvp = dot2d(vp, relative_model_matrix());
    FixedArray<float, 4, 4> m = dot2d(parent_m, relative_model_matrix());
    for(const auto& r : renderables_) {
        // The lightmap projection matrix is not perspective => mvp checks do not work.
        // => external_render_pass.black_node_name
        if ((!external_render_pass.black_node_name.empty() ||
                ((mvp(2, 3) > scene_graph_config.min_distance_small) &&
                (sum(squared(t3_from_4x4(mvp))) > squared(scene_graph_config.min_distance_small)) &&
                (sum(squared(t3_from_4x4(mvp))) < squared(scene_graph_config.max_distance_small)))) &&
            r.second->requires_blending_pass())
        {
            blended.push_back(Blended{mvp: mvp, m: m, renderable: r.second.get()});
        }
        r.second->render(mvp, m, iv, lights, scene_graph_config, render_config, {external_render_pass, InternalRenderPass::INITIAL});
    }
    for(const auto& n : children_) {
        n.second.second->render(mvp, m, iv, lights, blended, render_config, scene_graph_config, external_render_pass);
    }
}

void SceneNode::append_sorted_aggregates_to_queue(
    const FixedArray<float, 4, 4>& vp,
    const FixedArray<float, 4, 4>& parent_m,
    std::list<std::pair<float, std::shared_ptr<ColoredVertexArray>>>& aggregate_queue,
    const SceneGraphConfig& scene_graph_config) const
{
    // OpenGL matrices are transposed in memory,
    // https://stackoverflow.com/a/17718408/2292832.
    // "Note that post-multiplying with column-major matrices
    // produces the same result as pre-multiplying with
    // row-major matrices."
    FixedArray<float, 4, 4> mvp = dot2d(vp, relative_model_matrix());
    FixedArray<float, 4, 4> m = dot2d(parent_m, relative_model_matrix());
    for(const auto& r : renderables_) {
        r.second->append_sorted_aggregates_to_queue(mvp, m, scene_graph_config, aggregate_queue);
    }
    for(const auto& n : children_) {
        n.second.second->append_sorted_aggregates_to_queue(mvp, m, aggregate_queue, scene_graph_config);
    }
    for(const auto& a : aggregate_children_) {
        a.second.second->append_sorted_aggregates_to_queue(mvp, m, aggregate_queue, scene_graph_config);
    }
}

void SceneNode::append_large_aggregates_to_queue(
    const FixedArray<float, 4, 4>& parent_m,
    std::list<std::shared_ptr<ColoredVertexArray>>& aggregate_queue,
    const SceneGraphConfig& scene_graph_config) const
{
    FixedArray<float, 4, 4> m = dot2d(parent_m, relative_model_matrix());
    for(const auto& r : renderables_) {
        r.second->append_large_aggregates_to_queue(m, scene_graph_config, aggregate_queue);
    }
    for(const auto& n : children_) {
        n.second.second->append_large_aggregates_to_queue(m, aggregate_queue, scene_graph_config);
    }
    for(const auto& a : aggregate_children_) {
        a.second.second->append_large_aggregates_to_queue(m, aggregate_queue, scene_graph_config);
    }
}

void SceneNode::append_small_instances_to_queue(
    const FixedArray<float, 4, 4>& vp,
    const FixedArray<float, 4, 4>& parent_m,
    std::list<std::pair<float, TransformedColoredVertexArray>>& instances_queue,
    const SceneGraphConfig& scene_graph_config) const
{
    FixedArray<float, 4, 4> mvp = dot2d(vp, relative_model_matrix());
    FixedArray<float, 4, 4> m = dot2d(parent_m, relative_model_matrix());
    for(const auto& r : renderables_) {
        r.second->append_sorted_instances_to_queue(mvp, m, scene_graph_config, instances_queue);
    }
    for(const auto& n : children_) {
        n.second.second->append_small_instances_to_queue(mvp, m, instances_queue, scene_graph_config);
    }
    for(const auto& a : instances_children_) {
        a.second.second->append_small_instances_to_queue(mvp, m, instances_queue, scene_graph_config);
    }
}

void SceneNode::append_large_instances_to_queue(
    const FixedArray<float, 4, 4>& parent_m,
    std::list<TransformedColoredVertexArray>& instances_queue,
    const SceneGraphConfig& scene_graph_config) const
{
    FixedArray<float, 4, 4> m = dot2d(parent_m, relative_model_matrix());
    for(const auto& r : renderables_) {
        r.second->append_large_instances_to_queue(m, scene_graph_config, instances_queue);
    }
    for(const auto& n : children_) {
        n.second.second->append_large_instances_to_queue(m, instances_queue, scene_graph_config);
    }
    for(const auto& a : instances_children_) {
        a.second.second->append_large_instances_to_queue(m, instances_queue, scene_graph_config);
    }
}

void SceneNode::append_lights_to_queue(
    const FixedArray<float, 4, 4>& parent_m,
    std::list<std::pair<FixedArray<float, 4, 4>, Light*>>& lights) const
{
    FixedArray<float, 4, 4> m = dot2d(parent_m, relative_model_matrix());
    for(const auto& l : lights_) {
        lights.push_back(std::make_pair(m, l.get()));
    }
    for(const auto& n : children_) {
        n.second.second->append_lights_to_queue(m, lights);
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
    rotation_matrix_invalidated_ = true;
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

FixedArray<float, 4, 4> SceneNode::relative_model_matrix() const {
    if (rotation_matrix_invalidated_) {
        rotation_matrix_ = tait_bryan_angles_2_matrix(rotation_);
        rotation_matrix_invalidated_ = false;
    }
    return assemble_homogeneous_4x4(rotation_matrix_ * scale_, position_);
}

FixedArray<float, 4, 4> SceneNode::absolute_model_matrix() const {
    FixedArray<float, 4, 4> result = relative_model_matrix();
    if (parent_ != nullptr) {
        return dot2d(parent_->absolute_model_matrix(), result);
    } else {
        return result;
    }
}

FixedArray<float, 4, 4> SceneNode::relative_view_matrix() const {
    if (rotation_matrix_invalidated_) {
        rotation_matrix_ = tait_bryan_angles_2_matrix(rotation_);
        rotation_matrix_invalidated_ = false;
    }
    return assemble_inverse_homogeneous_4x4(rotation_matrix_ / scale_, position_);
}

FixedArray<float, 4, 4> SceneNode::absolute_view_matrix() const {
    FixedArray<float, 4, 4> result = relative_view_matrix();
    if (parent_ != nullptr) {
        return dot2d(result, parent_->absolute_view_matrix());
    } else {
        return result;
    }
}

void SceneNode::print(size_t recursion_depth) const {
    std::string rec(recursion_depth, '-');
    std::string rec2(recursion_depth + 1, '-');
    std::cerr << rec << " Position " << position() << std::endl;
    std::cerr << rec << " Rotation " << rotation() << std::endl;
    std::cout << rec << " Renderables" << std::endl;
    for(const auto& x : renderables_) {
        std::cerr << rec2 << " " << x.first << std::endl;
    }
    std::cout << rec << " Children" << std::endl;
    for(const auto& x : children_) {
        std::cerr << rec2 << " " << x.first << std::endl;
        x.second.second->print(recursion_depth + 1);
    }
    std::cout << rec << " Aggregate children" << std::endl;
    for(const auto& x : aggregate_children_) {
        std::cerr << rec2 << " " << x.first << std::endl;
        x.second.second->print(recursion_depth + 1);
    }
}
