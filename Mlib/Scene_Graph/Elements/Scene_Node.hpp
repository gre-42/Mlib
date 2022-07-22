#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Memory/Memory.hpp>
#include <Mlib/Scene_Graph/Elements/Camera.hpp>
#include <Mlib/Scene_Graph/Elements/Color_Style.hpp>
#include <Mlib/Scene_Graph/Elements/Light.hpp>
#include <Mlib/Scene_Graph/Elements/Renderable.hpp>
#include <Mlib/Scene_Graph/Transformation/Absolute_Movable.hpp>
#include <Mlib/Scene_Graph/Transformation/Absolute_Observer.hpp>
#include <Mlib/Scene_Graph/Transformation/Node_Modifier.hpp>
#include <Mlib/Scene_Graph/Transformation/Relative_Movable.hpp>
#include <map>
#include <memory>
#include <set>

namespace Mlib {

struct SceneGraphConfig;
struct RenderConfig;
class Scene;
class AnimationStateUpdater;
class SceneNodeResources;

struct Blended {
    int z_order;
    FixedArray<double, 4, 4> mvp;
    TransformationMatrix<float, double, 3> m;
    const Renderable* renderable;
    const AnimationState* animation_state;
    ColorStyle color_style;
    inline std::pair<int, float> sorting_key() const {
        return std::make_pair(z_order, mvp(2, 3));
    }
};

struct PositionAndYAngle {
    FixedArray<double, 3> position;
    float yangle;
    uint32_t billboard_id;
};

struct SceneNodeInstances {
    bool is_registered;
    std::unique_ptr<SceneNode> scene_node;
    std::list<PositionAndYAngle> instances;
};

struct SceneNodeChild {
    bool is_registered;
    std::unique_ptr<SceneNode> scene_node;
};

enum class SceneNodeState {
    DETACHED,
    STATIC,
    DYNAMIC
};

class SceneNode {
public:
    explicit SceneNode();
    SceneNode(const SceneNode& other) = delete;
    SceneNode& operator = (const SceneNode& other) = delete;
    ~SceneNode();
    bool shutting_down() const;
    AbsoluteMovable& get_absolute_movable() const;
    RelativeMovable& get_relative_movable() const;
    NodeModifier& get_node_modifier() const;
    AbsoluteObserver& get_absolute_observer() const;
    void set_absolute_movable(const observer_ptr<AbsoluteMovable>& absolute_movable);
    void set_relative_movable(const observer_ptr<RelativeMovable>& relative_movable);
    void set_node_modifier(std::unique_ptr<NodeModifier>&& node_modifier);
    void set_absolute_observer(const observer_ptr<AbsoluteObserver>& absolute_observer);
    void add_destruction_observer(
        DestructionObserver* destruction_observer,
        bool ignore_exists = false);
    void remove_destruction_observer(
        DestructionObserver* destruction_observer,
        bool ignore_not_exists = false);
    void add_renderable(
        const std::string& name,
        const std::shared_ptr<const Renderable>& renderable);
    void add_child(
        const std::string& name,
        std::unique_ptr<SceneNode>&& node,
        bool is_registered = false);
    SceneNode& parent();
    void clear_renderable_instance(const std::string& name);
    void clear_absolute_observer_and_notify_destroyed();
    SceneNode& get_child(const std::string& name) const;
    void remove_child(const std::string& name);
    bool contains_child(const std::string& name) const;
    void add_aggregate_child(
        const std::string& name,
        std::unique_ptr<SceneNode>&& node,
        bool is_registered = false);
    void add_instances_child(
        const std::string& name,
        std::unique_ptr<SceneNode>&& node,
        bool is_registered = false);
    void add_instances_position(
        const std::string& name,
        const FixedArray<double, 3>& position,
        float yangle,
        uint32_t billboard_id);
    bool has_camera() const;
    void set_camera(std::unique_ptr<Camera>&& camera);
    Camera& get_camera() const;
    void add_light(std::unique_ptr<Light>&& light);
    void move(
        const TransformationMatrix<float, double, 3>& v,
        float dt,
        SceneNodeResources* scene_node_resources,
        const AnimationState* animation_state);
    bool requires_render_pass(ExternalRenderPassType render_pass) const;
    void render(
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
        const std::list<const ColorStyle*>& color_styles) const;
    void append_sorted_aggregates_to_queue(
        const FixedArray<double, 4, 4>& vp,
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
        const FixedArray<double, 4, 4>& vp,
        const TransformationMatrix<float, double, 3>& parent_m,
        const FixedArray<double, 3>& offset,
        const PositionAndYAngle& delta_pose,
        SmallInstancesQueues& instances_queues,
        const SceneGraphConfig& scene_graph_config) const;
    void append_large_instances_to_queue(
        const FixedArray<double, 4, 4>& vp,
        const TransformationMatrix<float, double, 3>& parent_m,
        const FixedArray<double, 3>& offset,
        const PositionAndYAngle& delta_pose,
        LargeInstancesQueue& instances_queue,
        const SceneGraphConfig& scene_graph_config) const;
    void append_lights_to_queue(
        const TransformationMatrix<float, double, 3>& parent_m,
        std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights) const;
    const FixedArray<double, 3>& position() const;
    const FixedArray<float, 3>& rotation() const;
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
    TransformationMatrix<float, double, 3> absolute_model_matrix() const;
    TransformationMatrix<float, double, 3> relative_view_matrix() const;
    TransformationMatrix<float, double, 3> absolute_view_matrix() const;
    void print(std::ostream& ostr, size_t recursion_depth = 0) const;
    bool has_color_style(const std::string& name) const;
    ColorStyle& color_style(const std::string& name);
    void add_color_style(std::unique_ptr<ColorStyle>&& color_style);
    void set_animation_state(std::unique_ptr<AnimationState>&& animation_state);
    void set_animation_state_updater(std::unique_ptr<AnimationStateUpdater>&& animation_state_updater);
    bool to_be_deleted() const;
    void set_periodic_animation(const std::string& name);
    void set_aperiodic_animation(const std::string& name);
    void set_scene_and_state(Scene& scene, SceneNodeState state);
private:
    void setup_child(const std::string& name, SceneNode& node, bool is_registered);
    Scene* scene_;
    SceneNode* parent_;
    AbsoluteMovable* absolute_movable_;
    RelativeMovable* relative_movable_;
    std::unique_ptr<NodeModifier> node_modifier_;
    AbsoluteObserver* absolute_observer_;
    DestructionObserver* absolute_destruction_observer_;
    std::set<DestructionObserver*> destruction_observers_;
    std::unique_ptr<Camera> camera_;
    std::map<std::string, std::shared_ptr<const Renderable>> renderables_;
    std::map<std::string, SceneNodeChild> children_;
    std::map<std::string, SceneNodeChild> aggregate_children_;
    std::map<std::string, SceneNodeInstances> instances_children_;
    std::list<std::unique_ptr<Light>> lights_;
    FixedArray<double, 3> position_;
    FixedArray<float, 3> rotation_;
    float scale_;
    FixedArray<float, 3, 3> rotation_matrix_;
    std::unique_ptr<AnimationState> animation_state_;
    std::list<std::unique_ptr<ColorStyle>> color_styles_;
    std::unique_ptr<AnimationStateUpdater> animation_state_updater_;
    bool shutting_down_;
    std::string periodic_animation_;
    std::string aperiodic_animation_;
    SceneNodeState state_;
};

std::ostream& operator << (std::ostream& ostr, const SceneNode& node);

}
