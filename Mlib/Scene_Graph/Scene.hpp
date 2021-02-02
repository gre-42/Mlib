#pragma once
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
#include <Mlib/Threads/Background_Loop.hpp>
#include <atomic>
#include <iosfwd>
#include <map>
#include <memory>
#include <regex>
#include <thread>

namespace Mlib {

class AggregateRenderer;
class InstancesRenderer;

class Scene {
public:
    // Noncopyable because of mutex_
    Scene(
        AggregateRenderer* large_aggregate_renderer = nullptr,
        InstancesRenderer* large_instances_renderer = nullptr);
    Scene(const Scene&) = delete;
    Scene& operator = (const Scene&) = delete;
    ~Scene();
    bool contains_node(const std::string& name) const;
    void add_root_node(
        const std::string& name,
        SceneNode* scene_node);
    void add_static_root_node(
        const std::string& name,
        SceneNode* scene_node);
    void add_root_aggregate_node(
        const std::string& name,
        SceneNode* scene_node);
    void add_root_instances_node(
        const std::string& name,
        SceneNode* scene_node);
    bool root_node_scheduled_for_deletion(const std::string& name) const;
    void schedule_delete_root_node(const std::string& name);
    void delete_scheduled_root_nodes() const;
    void delete_root_node(const std::string& name);
    void delete_root_nodes(const Mlib::regex& regex);
    void register_node(
        const std::string& name,
        SceneNode* scene_node);
    void unregister_node(const std::string& name);
    void unregister_nodes(const Mlib::regex& regex);
    SceneNode* get_node(const std::string& name) const;
    void render(
        const FixedArray<float, 4, 4>& vp,
        const TransformationMatrix<float, 3>& iv,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        const ExternalRenderPass& external_render_pass,
        const std::function<std::function<void()>(std::function<void()>)>& run_in_background = [](std::function<void()> f){return f;}) const;
    void move(float dt);
    size_t get_uuid();
    void print(std::ostream& ostr) const;
    void shutdown();
    bool shutting_down() const;
private:
    // Must be above garbage-collected members
    // for deregistration in dtors to work.
    std::map<std::string, SceneNode*> nodes_;
    // |         |Lights|Blended|Large|Small|Move|
    // |---------|------|-------|-----|-----|----|
    // |Dynamic  |x     |x      |     |     |x   |
    // |Static   |x     |x      |x    |x    |    |
    // |Aggregate|      |       |x    |x    |    |
    // |Instances|      |       |x    |x    |    |
    std::map<std::string, std::unique_ptr<SceneNode>> root_nodes_;
    std::map<std::string, std::unique_ptr<SceneNode>> static_root_nodes_;
    std::map<std::string, std::unique_ptr<SceneNode>> root_aggregate_nodes_;
    std::map<std::string, std::unique_ptr<SceneNode>> root_instances_nodes_;
    AggregateRenderer* large_aggregate_renderer_;
    InstancesRenderer* large_instances_renderer_;
    mutable bool large_aggregate_renderer_initialized_;
    mutable bool large_instances_renderer_initialized_;
    mutable BackgroundLoop aggregation_bg_worker_;
    mutable BackgroundLoop instances_bg_worker_;
    std::mutex uuid_mutex_;
    size_t uuid_;
    bool shutting_down_;
    std::unique_ptr<const Style> style_;
    std::set<std::string> root_nodes_to_delete_;
    mutable std::mutex root_nodes_to_delete_mutex_;
};

std::ostream& operator << (std::ostream& ostr, const Scene& scene);

}
