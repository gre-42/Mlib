#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Memory.hpp>
#include <Mlib/Scene_Graph/Camera.hpp>
#include <Mlib/Scene_Graph/Light.hpp>
#include <Mlib/Scene_Graph/Renderable.hpp>
#include <Mlib/Scene_Graph/Transformation/Absolute_Movable.hpp>
#include <Mlib/Scene_Graph/Transformation/Absolute_Observer.hpp>
#include <Mlib/Scene_Graph/Transformation/Relative_Movable.hpp>
#include <map>
#include <memory>
#include <set>

namespace Mlib {

class SceneGraphConfig;
class RenderConfig;
class Scene;

struct Blended {
    FixedArray<float, 4, 4> mvp;
    FixedArray<float, 4, 4> m;
    const Renderable* renderable;
};

struct SceneNodeInstances {
    bool is_registered;
    std::unique_ptr<SceneNode> scene_node;
    std::list<FixedArray<float, 3>> instances;
};

struct SceneNodeChild {
    bool is_registered;
    std::unique_ptr<SceneNode> scene_node;
};

class SceneNode {
public:
    explicit SceneNode(Scene* scene = nullptr);
    SceneNode(const SceneNode& other) = delete;
    SceneNode& operator = (const SceneNode& other) = delete;
    ~SceneNode();
    void set_parent(SceneNode* parent);
    AbsoluteMovable* get_absolute_movable() const;
    RelativeMovable* get_relative_movable() const;
    AbsoluteObserver* get_absolute_observer() const;
    void set_absolute_movable(const observer_ptr<AbsoluteMovable>& absolute_movable);
    void set_relative_movable(const observer_ptr<RelativeMovable>& relative_movable);
    void set_absolute_observer(const observer_ptr<AbsoluteObserver>& absolute_observer);
    void add_destruction_observer(DestructionObserver* destruction_observer, bool ignore_exists = false);
    void remove_destruction_observer(DestructionObserver* destruction_observer);
    void add_renderable(
        const std::string& name,
        const std::shared_ptr<const Renderable>& renderable);
    void add_child(
        const std::string& name,
        SceneNode* node,
        bool is_registered = false);
    void add_aggregate_child(
        const std::string& name,
        SceneNode* node,
        bool is_registered = false);
    void add_instances_child(
        const std::string& name,
        SceneNode* node,
        bool is_registered = false);
    void add_instances_position(
        const std::string& name,
        const FixedArray<float, 3>& position);
    void set_camera(const std::shared_ptr<Camera>& camera);
    std::shared_ptr<Camera> get_camera() const;
    void add_light(Light* light);
    void move(const FixedArray<float, 4, 4>& v);
    bool requires_render_pass() const;
    void render(
        const FixedArray<float, 4, 4>& vp,
        const FixedArray<float, 4, 4>& parent_m,
        const FixedArray<float, 4, 4>& iv,
        const std::list<std::pair<FixedArray<float, 4, 4>, Light*>>& lights,
        std::list<Blended>& blended,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        ExternalRenderPass external_render_pass) const;
    void append_sorted_aggregates_to_queue(
        const FixedArray<float, 4, 4>& vp,
        const FixedArray<float, 4, 4>& parent_m,
        std::list<std::pair<float, std::shared_ptr<ColoredVertexArray>>>& aggregate_queue,
        const SceneGraphConfig& scene_graph_config,
        ExternalRenderPass external_render_pass) const;
    void append_large_aggregates_to_queue(
        const FixedArray<float, 4, 4>& parent_m,
        std::list<std::shared_ptr<ColoredVertexArray>>& aggregate_queue,
        const SceneGraphConfig& scene_graph_config) const;
    void append_small_instances_to_queue(
        const FixedArray<float, 4, 4>& vp,
        const FixedArray<float, 4, 4>& parent_m,
        const FixedArray<float, 3>& delta_position,
        std::list<std::pair<float, TransformedColoredVertexArray>>& instances_queue,
        const SceneGraphConfig& scene_graph_config,
        ExternalRenderPass external_render_pass) const;
    void append_large_instances_to_queue(
        const FixedArray<float, 4, 4>& parent_m,
        const FixedArray<float, 3>& delta_position,
        std::list<TransformedColoredVertexArray>& instances_queue,
        const SceneGraphConfig& scene_graph_config) const;
    void append_lights_to_queue(
        const FixedArray<float, 4, 4>& parent_m,
        std::list<std::pair<FixedArray<float, 4, 4>, Light*>>& lights) const;
    const FixedArray<float, 3>& position() const;
    const FixedArray<float, 3>& rotation() const;
    float scale() const;
    void set_position(const FixedArray<float, 3>& position);
    void set_rotation(const FixedArray<float, 3>& rotation);
    void set_scale(float scale);
    void set_relative_pose(
        const FixedArray<float, 3>& position,
        const FixedArray<float, 3>& rotation,
        float scale);
    FixedArray<float, 4, 4> relative_model_matrix() const;
    FixedArray<float, 4, 4> absolute_model_matrix() const;
    FixedArray<float, 4, 4> relative_view_matrix() const;
    FixedArray<float, 4, 4> absolute_view_matrix() const;
    void print(size_t recursion_depth) const;
private:
    Scene* scene_;
    SceneNode* parent_;
    AbsoluteMovable* absolute_movable_;
    RelativeMovable* relative_movable_;
    AbsoluteObserver* absolute_observer_;
    std::set<DestructionObserver*> destruction_observers_;
    std::shared_ptr<Camera> camera_;
    std::map<std::string, std::shared_ptr<const Renderable>> renderables_;
    std::map<std::string, SceneNodeChild> children_;
    std::map<std::string, SceneNodeChild> aggregate_children_;
    std::map<std::string, SceneNodeInstances> instances_children_;
    std::list<std::unique_ptr<Light>> lights_;
    FixedArray<float, 3> position_;
    FixedArray<float, 3> rotation_;
    float scale_;
    mutable FixedArray<float, 3, 3> rotation_matrix_;
    mutable bool rotation_matrix_invalidated_;

};

}
