#include "Scene_Node.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <Mlib/Scene_Graph/Style.hpp>

using namespace Mlib;

SceneNode::SceneNode(Scene* scene)
: scene_{scene},
  parent_{nullptr},
  absolute_movable_{nullptr},
  relative_movable_{nullptr},
  absolute_observer_{nullptr},
  position_{0.f, 0.f, 0.f},
  rotation_{0.f, 0.f, 0.f},
  scale_{1.f},
  rotation_matrix_{fixed_identity_array<float, 3>()}
{}

SceneNode::~SceneNode() {
    for (auto& o : destruction_observers_) {
        o->notify_destroyed(this);
    }
    for (auto& c : children_) {
        if (c.second.is_registered) {
            // scene_ is non-null, checked in "add_child".
            scene_->unregister_node(c.first);
        }
    }
    for (auto& c : aggregate_children_) {
        if (c.second.is_registered) {
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

void SceneNode::add_destruction_observer(DestructionObserver* destruction_observer, bool ignore_exists) {
    auto r = destruction_observers_.insert(destruction_observer);
    if (!r.second && !ignore_exists) {
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
    const std::shared_ptr<const Renderable>& renderable)
{
    if (name.empty()) {
        throw std::runtime_error("Renderable has no name");
    }
    if (!renderables_.insert(std::make_pair(name, renderable)).second) {
        throw std::runtime_error("Renderable with name " + name + " already exists");
    }
}

void SceneNode::add_child(
    const std::string& name,
    SceneNode* node,
    bool is_registered)
{
    // Required in SceneNonde::~SceneNode
    if (is_registered && (scene_ == nullptr)) {
        throw std::runtime_error("Parent of registered node " + name + " does not have a scene");
    }
    if (name.empty()) {
        throw std::runtime_error("Child node has no name");
    }
    if (!children_.insert(std::make_pair(name, SceneNodeChild{
        .is_registered = is_registered,
        .scene_node = std::unique_ptr<SceneNode>(node)})).second)
    {
        throw std::runtime_error("Child node with name " + name + " already exists");
    }
}

void SceneNode::add_aggregate_child(
    const std::string& name,
    SceneNode* node,
    bool is_registered)
{
    // Required in SceneNonde::~SceneNode
    if (is_registered && (scene_ == nullptr)) {
        throw std::runtime_error("Parent of registered node " + name + " does not have a scene");
    }
    if (name.empty()) {
        throw std::runtime_error("Child node has no name");
    }
    if (!aggregate_children_.insert(std::make_pair(name, SceneNodeChild{
        .is_registered = is_registered,
        .scene_node = std::unique_ptr<SceneNode>(node)})).second)
    {
        throw std::runtime_error("Aggregate node with name " + name + " already exists");
    }
}

void SceneNode::add_instances_child(
    const std::string& name,
    SceneNode* node,
    bool is_registered)
{
    // Required in SceneNonde::~SceneNode
    if (is_registered && (scene_ == nullptr)) {
        throw std::runtime_error("Parent of registered node " + name + " does not have a scene");
    }
    if (name.empty()) {
        throw std::runtime_error("Child node has no name");
    }
    if (!instances_children_.insert(std::make_pair(name, SceneNodeInstances{
        .is_registered = is_registered,
        .scene_node = std::move(std::unique_ptr<SceneNode>{node}),
        .instances = std::move(std::list<FixedArray<float, 3>>())})).second)
    {
        throw std::runtime_error("Aggregate node with name " + name + " already exists");
    }
}

void SceneNode::add_instances_position(
    const std::string& name,
    const FixedArray<float, 3>& position)
{
    auto it = instances_children_.find(name);
    if (it == instances_children_.end()) {
        throw std::runtime_error("Could not find instance node with name " + name);
    }
    it->second.instances.push_back(position);
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

void SceneNode::set_style(Style* style) {
    if (!renderables_.empty()) {
        throw std::runtime_error("Style was set after renderables, this leads to a race condition");
    }
    style_.reset(style);
}

void SceneNode::move(const TransformationMatrix<float, 3>& v, float dt) {
    if (style_ != nullptr) {
        AnimationFrame& af = style_->animation_frame;
        if (!std::isnan(af.loop_time)) {
            if (std::isnan(af.loop_begin) != std::isnan(af.loop_end)) {
                throw std::runtime_error("Inconsistent loop_begin and loop_end NAN-ness");
            }
            if (!std::isnan(af.loop_begin)) {
                if (af.loop_end < af.loop_begin) {
                    throw std::runtime_error("Loop end before loop begin");
                }
                if (af.loop_time < af.loop_begin) {
                    throw std::runtime_error("Loop time before loop begin");
                }
                if (af.loop_time > af.loop_end) {
                    throw std::runtime_error("Loop time after loop end");
                }
                if (af.loop_end == af.loop_begin) {
                    af.loop_time = af.loop_begin;
                } else {
                    af.loop_time = std::clamp(
                        af.loop_begin + std::fmod(
                            af.loop_time + dt - af.loop_begin,
                            af.loop_end - af.loop_begin),
                        af.loop_begin,
                        af.loop_end);
                }
            }
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
    for (const auto& n : children_) {
        n.second.scene_node->move(v2, dt);
    }
}

bool SceneNode::requires_render_pass() const {
    for (const auto& r : renderables_) {
        if (r.second->requires_render_pass()) {
            return true;
        }
    }
    for (const auto& n : children_) {
        if (n.second.scene_node->requires_render_pass()) {
            return true;
        }
    }
    return false;
}

void SceneNode::render(
    const FixedArray<float, 4, 4>& vp,
    const TransformationMatrix<float, 3>& parent_m,
    const TransformationMatrix<float, 3>& iv,
    const std::list<std::pair<TransformationMatrix<float, 3>, Light*>>& lights,
    std::list<Blended>& blended,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    const ExternalRenderPass& external_render_pass,
    const Style* style) const
{
    // OpenGL matrices are transposed in memory,
    // https://stackoverflow.com/a/17718408/2292832.
    // "Note that post-multiplying with column-major matrices
    // produces the same result as pre-multiplying with
    // row-major matrices."
    FixedArray<float, 4, 4> mvp = dot2d(vp, relative_model_matrix().affine());
    auto m = parent_m * relative_model_matrix();
    const Style* estyle = style_ != nullptr
        ? style_.get()
        : style;
    for (const auto& r : renderables_) {
        if (r.second->requires_blending_pass())
        {
            blended.push_back(Blended{.z_order = r.second->continuous_blending_z_order(), .mvp = mvp, .m = m, .renderable = r.second.get()});
        }
        r.second->render(
            mvp,
            m,
            iv,
            lights,
            scene_graph_config,
            render_config,
            {external_render_pass, InternalRenderPass::INITIAL},
            (estyle != nullptr) && Mlib::re::regex_search(r.first, estyle->selector)
                ? estyle
                : nullptr);
    }
    for (const auto& n : children_) {
        n.second.scene_node->render(
            mvp,
            m,
            iv,
            lights,
            blended,
            render_config,
            scene_graph_config,
            external_render_pass,
            estyle);
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
    for (const auto& r : renderables_) {
        r.second->append_sorted_aggregates_to_queue(mvp, m, scene_graph_config, external_render_pass, aggregate_queue);
    }
    for (const auto& n : children_) {
        n.second.scene_node->append_sorted_aggregates_to_queue(mvp, m, aggregate_queue, scene_graph_config, external_render_pass);
    }
    for (const auto& a : aggregate_children_) {
        a.second.scene_node->append_sorted_aggregates_to_queue(mvp, m, aggregate_queue, scene_graph_config, external_render_pass);
    }
}

void SceneNode::append_large_aggregates_to_queue(
    const TransformationMatrix<float, 3>& parent_m,
    std::list<std::shared_ptr<ColoredVertexArray>>& aggregate_queue,
    const SceneGraphConfig& scene_graph_config) const
{
    TransformationMatrix<float, 3> m = parent_m * relative_model_matrix();
    for (const auto& r : renderables_) {
        r.second->append_large_aggregates_to_queue(m, scene_graph_config, aggregate_queue);
    }
    for (const auto& n : children_) {
        n.second.scene_node->append_large_aggregates_to_queue(m, aggregate_queue, scene_graph_config);
    }
    for (const auto& a : aggregate_children_) {
        a.second.scene_node->append_large_aggregates_to_queue(m, aggregate_queue, scene_graph_config);
    }
}

void SceneNode::append_small_instances_to_queue(
    const FixedArray<float, 4, 4>& vp,
    const TransformationMatrix<float, 3>& parent_m,
    const FixedArray<float, 3>& delta_position,
    std::list<std::pair<float, TransformedColoredVertexArray>>& instances_queue,
    const SceneGraphConfig& scene_graph_config,
    const ExternalRenderPass& external_render_pass) const
{
    TransformationMatrix<float, 3> rel = relative_model_matrix();
    rel.t() += delta_position;
    FixedArray<float, 4, 4> mvp = dot2d(vp, rel.affine());
    TransformationMatrix<float, 3> m = parent_m * rel;
    for (const auto& r : renderables_) {
        r.second->append_sorted_instances_to_queue(mvp, m, scene_graph_config, external_render_pass, instances_queue);
    }
    for (const auto& n : children_) {
        n.second.scene_node->append_small_instances_to_queue(mvp, m, fixed_zeros<float, 3>(), instances_queue, scene_graph_config, external_render_pass);
    }
    for (const auto& i : instances_children_) {
        // Nested loop because each instance position is to
        // be handled like a child node, not a renderable.
        for (const auto& j : i.second.instances) {
            i.second.scene_node->append_small_instances_to_queue(mvp, m, j, instances_queue, scene_graph_config, external_render_pass);
        }
    }
}

void SceneNode::append_large_instances_to_queue(
    const TransformationMatrix<float, 3>& parent_m,
    const FixedArray<float, 3>& delta_position,
    std::list<TransformedColoredVertexArray>& instances_queue,
    const SceneGraphConfig& scene_graph_config) const
{
    TransformationMatrix<float, 3> rel = relative_model_matrix();
    rel.t() += delta_position;
    TransformationMatrix<float, 3> m = parent_m * rel;
    for (const auto& r : renderables_) {
        r.second->append_large_instances_to_queue(m, scene_graph_config, instances_queue);
    }
    for (const auto& n : children_) {
        n.second.scene_node->append_large_instances_to_queue(m, fixed_zeros<float, 3>(), instances_queue, scene_graph_config);
    }
    for (const auto& i : instances_children_) {
        for (const auto& j : i.second.instances) {
            // Nested loop because each instance position is to
            // be handled like a child node, not a renderable.
            i.second.scene_node->append_large_instances_to_queue(m, j, instances_queue, scene_graph_config);
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
    for (const auto& n : children_) {
        n.second.scene_node->append_lights_to_queue(m, lights);
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
        ostr << " " << ind1 << " Renderables (" << renderables_.size() << ")\n";
        for (const auto& r : renderables_) {
            ostr << " " << ind2 << " " << r.first << '\n';
        }
    }
    if (!children_.empty()) {
        ostr << " " << ind1 << " Children (" << children_.size() << ")\n";
        for (const auto& n : children_) {
            ostr << " " << ind2 << " " << n.first << '\n';
            n.second.scene_node->print(ostr, recursion_depth + 1);
        }
    }
    if (!aggregate_children_.empty()) {
        ostr << " " << ind1 << " Aggregates (" << aggregate_children_.size() << ")\n";
        for (const auto& n : aggregate_children_) {
            ostr << " " << ind2 << " " << n.first << '\n';
            n.second.scene_node->print(ostr, recursion_depth + 1);
        }
    }
    if (!instances_children_.empty()) {
        ostr << " " << ind1 << " Instances (" << instances_children_.size() << ")\n";
        for (const auto& n : instances_children_) {
            ostr << " " << ind2 << " " << n.first << " n=" << n.second.instances.size() << '\n';
            n.second.scene_node->print(ostr, recursion_depth + 1);
        }
    }
    ostr << " " << ind0 << " End\n";
}

std::ostream& Mlib::operator << (std::ostream& ostr, const SceneNode& node) {
    node.print(ostr);
    return ostr;
}
