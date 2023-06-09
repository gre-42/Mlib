#pragma once
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene_Graph/Containers/Map_Of_Root_Nodes.hpp>
#include <Mlib/Threads/Background_Loop.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <atomic>
#include <iosfwd>
#include <list>
#include <memory>
#include <set>
#include <thread>

namespace Mlib {

class DeleteNodeMutex;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
class SceneNode;
class SceneNodeResources;
class IParticleRenderer;
class IParticleInstantiator;
struct SceneGraphConfig;
struct ExternalRenderPass;
struct RenderConfig;
struct ColorStyle;

class Scene {
    friend RootNodes;
public:
    // Noncopyable because of mutex_
    explicit Scene(
        DeleteNodeMutex& delete_node_mutex,
        SceneNodeResources* scene_node_resources = nullptr,
        IParticleRenderer* particle_renderer = nullptr);
    Scene(const Scene&) = delete;
    Scene& operator = (const Scene&) = delete;
    ~Scene();
    bool contains_node(const std::string& name) const;
    void add_root_node(
        const std::string& name,
        std::unique_ptr<SceneNode>&& scene_node);
    void add_static_root_node(
        const std::string& name,
        std::unique_ptr<SceneNode>&& scene_node);
    void add_root_aggregate_node(
        const std::string& name,
        std::unique_ptr<SceneNode>&& scene_node);
    void add_root_instances_node(
        const std::string& name,
        std::unique_ptr<SceneNode>&& scene_node);
    void add_root_imposter_node(SceneNode* scene_node);
    bool root_node_scheduled_for_deletion(
        const std::string& name,
        bool must_exist = true) const;
    void schedule_delete_root_node(const std::string& name);
    void delete_scheduled_root_nodes() const;
    void try_delete_root_node(const std::string& name);
    void delete_root_imposter_node(SceneNode& scene_node);
    void delete_root_node(const std::string& name);
    void delete_root_nodes(const Mlib::regex& regex);
    void try_delete_node(const std::string& name);
    void delete_node(const std::string& name);
    void delete_nodes(const Mlib::regex& regex);
    void register_node(
        const std::string& name,
        SceneNode& scene_node);
    void unregister_node(const std::string& name);
    void unregister_nodes(const Mlib::regex& regex);
    SceneNode& get_node(const std::string& name) const;
    std::list<std::pair<std::string, SceneNode&>> get_nodes(const Mlib::regex& regex) const;
    void render(
        const FixedArray<double, 4, 4>& vp,
        const TransformationMatrix<float, double, 3>& iv,
        const SceneNode& camera_node,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        const ExternalRenderPass& external_render_pass,
        const std::function<std::function<void()>(std::function<void()>)>& run_in_background = [](std::function<void()> f){return f;}) const;
    void move(float dt);
    size_t get_uuid();
    std::string get_temporary_instance_suffix();
    void print(std::ostream& ostr) const;
    void shutdown();
    bool shutting_down() const;
    void add_node_not_allowed_to_be_unregistered(const std::string& name);
    void remove_node_not_allowed_to_be_unregistered(const std::string& name);
    void clear_nodes_not_allowed_to_be_unregistered();
    void add_color_style(std::unique_ptr<ColorStyle>&& color_style);
    DeleteNodeMutex& delete_node_mutex() const;
    IParticleInstantiator& particle_instantiator(const std::string& resource_name) const;
private:
    SceneNode& get_node_that_may_be_scheduled_for_deletion(const std::string& name) const;
    // Must be above garbage-collected members for
    // deregistration of child nodes in SceneNode
    // dtor to work.
    std::map<std::string, SceneNode*> nodes_;
    // |         |Lights|Blended|Large|Small|Move|
    // |---------|------|-------|-----|-----|----|
    // |Dynamic  |x     |x      |     |     |x   |
    // |Static   |x     |x      |x    |x    |    |
    // |Aggregate|      |       |x    |x    |    |
    // |Instances|      |       |x    |x    |    |
    MapOfRootNodes morn_;
    RootNodes& root_nodes_;
    RootNodes& static_root_nodes_;
    RootNodes& root_aggregate_nodes_;
    RootNodes& root_instances_nodes_;
    std::set<SceneNode*> root_imposter_nodes_;
    DeleteNodeMutex& delete_node_mutex_;
    mutable RecursiveSharedMutex mutex_;
    mutable BackgroundLoop large_aggregate_bg_worker_;
    mutable BackgroundLoop large_instances_bg_worker_;
    mutable BackgroundLoop small_aggregate_bg_worker_;
    mutable BackgroundLoop small_instances_bg_worker_;
    std::mutex uuid_mutex_;
    size_t uuid_;
    bool shutting_down_;
    std::list<std::unique_ptr<const ColorStyle>> color_styles_;
    std::set<std::string> nodes_not_allowed_to_be_unregistered_;
    SceneNodeResources* scene_node_resources_;
    IParticleRenderer* particle_renderer_;
};

std::ostream& operator << (std::ostream& ostr, const Scene& scene);

}
