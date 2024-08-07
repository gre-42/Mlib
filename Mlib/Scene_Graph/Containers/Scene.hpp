#pragma once
#include <Mlib/Math/Time_Point_Series.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene_Graph/Containers/Map_Of_Root_Nodes.hpp>
#include <Mlib/Scene_Graph/Interpolation.hpp>
#include <Mlib/Scene_Pos.hpp>
#include <Mlib/Threads/Atomic_Mutex.hpp>
#include <Mlib/Threads/Background_Loop.hpp>
#include <Mlib/Threads/Safe_Recursive_Shared_Mutex.hpp>
#include <atomic>
#include <functional>
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
class ITrailRenderer;
class IDynamicLights;
class IParticleCreator;
struct SceneGraphConfig;
struct ExternalRenderPass;
struct RenderConfig;
struct ColorStyle;
class Renderable;
struct ColoredVertexArrayFilter;
template <class TPos>
class ColoredVertexArray;
enum class RenderingDynamics;
enum class RenderingStrategies;

class Scene {
    friend RootNodes;
public:
    // Noncopyable because of mutex_
    explicit Scene(
        DeleteNodeMutex& delete_node_mutex,
        SceneNodeResources* scene_node_resources = nullptr,
        IParticleRenderer* particle_renderer = nullptr,
        ITrailRenderer* trail_renderer = nullptr,
        IDynamicLights* dynamic_lights = nullptr);
    Scene(const Scene&) = delete;
    Scene& operator = (const Scene&) = delete;
    ~Scene();
    bool contains_node(const std::string& name) const;
    void add_moving_root_node(
        const std::string& name,
        DanglingUniquePtr<SceneNode>&& scene_node);
    void add_static_root_node(
        const std::string& name,
        DanglingUniquePtr<SceneNode>&& scene_node);
    void add_root_aggregate_once_node(
        const std::string& name,
        DanglingUniquePtr<SceneNode>&& scene_node);
    void add_root_aggregate_always_node(
        const std::string& name,
        DanglingUniquePtr<SceneNode>&& scene_node);
    void add_root_instances_once_node(
        const std::string& name,
        DanglingUniquePtr<SceneNode>&& scene_node);
    void add_root_instances_always_node(
        const std::string& name,
        DanglingUniquePtr<SceneNode>&& scene_node);
    void auto_add_root_node(
        const std::string& name,
        DanglingUniquePtr<SceneNode>&& scene_node,
        RenderingDynamics rendering_dynamics);
    void add_root_node(
        const std::string& name,
        DanglingUniquePtr<SceneNode>&& scene_node,
        RenderingDynamics rendering_dynamics,
        RenderingStrategies rendering_strategy);
    void add_root_imposter_node(DanglingRef<SceneNode> scene_node);
    bool root_node_scheduled_for_deletion(
        const std::string& name,
        bool must_exist = true) const;
    void schedule_delete_root_node(const std::string& name);
    void delete_scheduled_root_nodes() const;
    void try_delete_root_node(const std::string& name);
    void delete_root_imposter_node(DanglingRef<SceneNode> scene_node);
    void delete_root_node(const std::string& name);
    void delete_root_nodes(const Mlib::regex& regex);
    void try_delete_node(const std::string& name);
    void delete_node(const std::string& name);
    void delete_nodes(const Mlib::regex& regex);
    void register_node(
        const std::string& name,
        DanglingRef<SceneNode> scene_node);
    void unregister_node(const std::string& name);
    void unregister_nodes(const Mlib::regex& regex);
    DanglingRef<SceneNode> get_node(const std::string& name, SOURCE_LOCATION loc) const;
    std::list<std::pair<std::string, DanglingRef<SceneNode>>> get_nodes(const Mlib::regex& regex) const;
    bool visit_all(const std::function<bool(
        const TransformationMatrix<float, ScenePos, 3>& m,
        const std::map<std::string, std::shared_ptr<const Renderable>>& renderables)>& func) const;
    void render(
        const FixedArray<ScenePos, 4, 4>& vp,
        const TransformationMatrix<float, ScenePos, 3>& iv,
        const DanglingRef<const SceneNode>& camera_node,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        const ExternalRenderPass& external_render_pass,
        const std::function<std::function<void()>(std::function<void()>)>& run_in_background = [](std::function<void()> f){return f;}) const;
    void move(float dt, std::chrono::steady_clock::time_point time);
    void append_static_filtered_to_queue(
        std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<ColoredVertexArray<float>>>>& float_queue,
        std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<ColoredVertexArray<double>>>>& double_queue,
        const ColoredVertexArrayFilter& filter) const;
    size_t get_uuid();
    std::string get_temporary_instance_suffix();
    void print(std::ostream& ostr) const;
    void shutdown();
    bool shutting_down() const;
    void stop_and_join();
    void wait_until_done() const;
    void add_node_not_allowed_to_be_unregistered(const std::string& name);
    void remove_node_not_allowed_to_be_unregistered(const std::string& name);
    void clear_nodes_not_allowed_to_be_unregistered();
    void add_color_style(std::unique_ptr<ColorStyle>&& color_style);
    DeleteNodeMutex& delete_node_mutex() const;
    IParticleCreator& particle_instantiator(const std::string& resource_name) const;
private:
    DanglingRef<SceneNode> get_node_that_may_be_scheduled_for_deletion(const std::string& name) const;
    // Must be above garbage-collected members for
    // deregistration of child nodes in SceneNode
    // dtor to work.
    std::map<std::string, DanglingPtr<SceneNode>> nodes_;
    // |         |Lights|Blended|Large|Small|Move|
    // |---------|------|-------|-----|-----|----|
    // |Dynamic  |x     |x      |     |     |x   |
    // |Static   |x     |x      |x    |x    |    |
    // |Aggregate|      |       |x    |x    |    |
    // |Instances|      |       |x    |x    |    |
    MapOfRootNodes morn_;
    RootNodes& root_nodes_;
    RootNodes& static_root_nodes_;
    RootNodes& root_aggregate_once_nodes_;
    RootNodes& root_aggregate_always_nodes_;
    RootNodes& root_instances_once_nodes_;
    RootNodes& root_instances_always_nodes_;
    std::set<DanglingPtr<SceneNode>> root_imposter_nodes_;
    DeleteNodeMutex& delete_node_mutex_;
    mutable SafeAtomicRecursiveSharedMutex mutex_;
    mutable BackgroundLoop large_aggregate_bg_worker_;
    mutable BackgroundLoop large_instances_bg_worker_;
    mutable BackgroundLoop small_aggregate_bg_worker_;
    mutable BackgroundLoop small_instances_bg_worker_;
    AtomicMutex uuid_mutex_;
    size_t uuid_;
    std::atomic_bool shutting_down_;
    std::list<std::unique_ptr<const ColorStyle>> color_styles_;
    std::set<std::string> nodes_not_allowed_to_be_unregistered_;
    SceneNodeResources* scene_node_resources_;
    IParticleRenderer* particle_renderer_;
    ITrailRenderer* trail_renderer_;
    IDynamicLights* dynamic_lights_;
    TimePointSeries<NINTERPOLATED> times_;
};

std::ostream& operator << (std::ostream& ostr, const Scene& scene);

}
