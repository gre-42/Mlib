#pragma once
#include <Mlib/List/Thread_Safe_List.hpp>
#include <Mlib/Map/String_With_Hash_Unordered_Map.hpp>
#include <Mlib/Math/Time_Point_Series.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene_Graph/Containers/Map_Of_Root_Nodes.hpp>
#include <Mlib/Scene_Graph/Interpolation.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Threads/Background_Loop.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <atomic>
#include <functional>
#include <iosfwd>
#include <list>
#include <memory>
#include <set>
#include <thread>
#include <unordered_map>

namespace Mlib {

class DeleteNodeMutex;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
class SceneNode;
class SceneNodeResources;
class IRenderableScene;
class ITrailRenderer;
class IDynamicLights;
struct SceneGraphConfig;
struct ExternalRenderPass;
struct RenderConfig;
struct ColorStyle;
class Renderable;
template <class TPos>
class ColoredVertexArray;
enum class RenderingDynamics;
enum class RenderingStrategies;
class RenderableWithStyle;
template <class T>
class VariableAndHash;

class Scene {
    friend RootNodes;
public:
    // Noncopyable because of mutex_
    explicit Scene(
        std::string name,
        DeleteNodeMutex& delete_node_mutex,
        SceneNodeResources* scene_node_resources = nullptr,
        ITrailRenderer* trail_renderer = nullptr,
        IDynamicLights* dynamic_lights = nullptr);
    Scene(const Scene&) = delete;
    Scene& operator = (const Scene&) = delete;
    ~Scene();
    void add_to_trash_can(DanglingUniquePtr<SceneNode>&& node);
    void add_to_trash_can(std::unique_ptr<DanglingBaseClass>&& obj);
    size_t try_empty_the_trash_can();
    bool contains_node(const VariableAndHash<std::string>& name) const;
    void add_moving_root_node(
        const VariableAndHash<std::string>& name,
        DanglingUniquePtr<SceneNode>&& scene_node);
    void add_static_root_node(
        const VariableAndHash<std::string>& name,
        DanglingUniquePtr<SceneNode>&& scene_node);
    void add_root_aggregate_once_node(
        const VariableAndHash<std::string>& name,
        DanglingUniquePtr<SceneNode>&& scene_node);
    void add_root_aggregate_always_node(
        const VariableAndHash<std::string>& name,
        DanglingUniquePtr<SceneNode>&& scene_node);
    void add_root_instances_once_node(
        const VariableAndHash<std::string>& name,
        DanglingUniquePtr<SceneNode>&& scene_node);
    void add_root_instances_always_node(
        const VariableAndHash<std::string>& name,
        DanglingUniquePtr<SceneNode>&& scene_node);
    void add_static_root_physics_node(
        const VariableAndHash<std::string>& name,
        DanglingUniquePtr<SceneNode>&& scene_node);
    void auto_add_root_node(
        const VariableAndHash<std::string>& name,
        DanglingUniquePtr<SceneNode>&& scene_node,
        RenderingDynamics rendering_dynamics);
    void add_root_node(
        const VariableAndHash<std::string>& name,
        DanglingUniquePtr<SceneNode>&& scene_node,
        RenderingDynamics rendering_dynamics,
        RenderingStrategies rendering_strategy);
    void add_root_imposter_node(
        const DanglingBaseClassPtr<IRenderableScene>& renderable_scene,
        const DanglingRef<SceneNode>& scene_node);
    void move_root_node_to_bvh(const VariableAndHash<std::string>& name);
    bool root_node_scheduled_for_deletion(
        const VariableAndHash<std::string>& name,
        bool must_exist = true) const;
    void schedule_delete_root_node(const VariableAndHash<std::string>& name);
    void delete_scheduled_root_nodes() const;
    void try_delete_root_node(const VariableAndHash<std::string>& name);
    void delete_root_imposter_node(
        const DanglingBaseClassPtr<IRenderableScene>& renderable_scene,
        const DanglingRef<SceneNode>& scene_node);
    void delete_root_node(const VariableAndHash<std::string>& name);
    void delete_root_nodes(const Mlib::re::cregex& regex);
    void try_delete_node(const VariableAndHash<std::string>& name);
    void delete_node(const VariableAndHash<std::string>& name);
    void delete_nodes(const Mlib::re::cregex& regex);
    void register_node(
        const VariableAndHash<std::string>& name,
        const DanglingRef<SceneNode>& scene_node);
    void unregister_node(const VariableAndHash<std::string>& name);
    void unregister_nodes(const Mlib::re::cregex& regex);
    DanglingRef<SceneNode> get_node(const VariableAndHash<std::string>& name, SOURCE_LOCATION loc) const;
    DanglingPtr<SceneNode> try_get_node(const VariableAndHash<std::string>& name, SOURCE_LOCATION loc) const;
    std::list<std::pair<VariableAndHash<std::string>, DanglingRef<SceneNode>>> get_nodes(const Mlib::re::cregex& regex) const;
    bool visit_all(const std::function<bool(
        const TransformationMatrix<float, ScenePos, 3>& m,
        const StringWithHashUnorderedMap<std::shared_ptr<RenderableWithStyle>>& renderables)>& func) const;
    void render(
        const FixedArray<ScenePos, 4, 4>& vp,
        const TransformationMatrix<float, ScenePos, 3>& iv,
        const DanglingPtr<const SceneNode>& camera_node,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        const ExternalRenderPass& external_render_pass,
        const std::function<std::function<void()>(std::function<void()>)>& run_in_background = [](std::function<void()> f){return f;}) const;
    void move(float dt, std::chrono::steady_clock::time_point time);
    void append_physics_to_queue(
        std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<ColoredVertexArray<float>>>>& float_queue,
        std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>>& double_queue) const;
    size_t get_uuid();
    std::string get_temporary_instance_suffix();
    void print(std::ostream& ostr) const;
    void shutdown();
    bool shutting_down() const;
    void stop_and_join();
    void wait_until_done() const;
    void add_color_style(std::unique_ptr<ColorStyle>&& color_style);
    void wait_for_cleanup() const;
    void notify_cleanup_required();
    void notify_cleanup_done();
    DeleteNodeMutex& delete_node_mutex() const;
    void set_this_thread_as_render_thread();
    void clear_render_thread();
    void assert_this_thread_is_render_thread() const;
private:
    DanglingRef<SceneNode> get_node_that_may_be_scheduled_for_deletion(const VariableAndHash<std::string>& name) const;
    // Must be above garbage-collected members for
    // deregistration of child nodes in SceneNode
    // dtor to work.
    StringWithHashUnorderedMap<DanglingPtr<SceneNode>> nodes_;
    // |         |Lights|Blended|Large|Small|Move|
    // |---------|------|-------|-----|-----|----|
    // |Dynamic  |x     |x      |     |     |x   |
    // |Static   |x     |x      |x    |x    |    |
    // |Aggregate|      |       |x    |x    |    |
    // |Instances|      |       |x    |x    |    |
    DeleteNodeMutex& delete_node_mutex_;
    std::thread::id render_thread_id_;
    MapOfRootNodes morn_;
    RootNodes& root_nodes_;
    RootNodes& static_root_nodes_;
    RootNodes& root_aggregate_once_nodes_;
    RootNodes& root_aggregate_always_nodes_;
    RootNodes& root_instances_once_nodes_;
    RootNodes& root_instances_always_nodes_;
    RootNodes& static_root_physics_nodes_;
    std::unordered_map<
        DanglingBaseClassPtr<IRenderableScene>,
        std::set<DanglingPtr<SceneNode>>> root_imposter_nodes_;
    std::string name_;
    mutable SafeAtomicRecursiveSharedMutex mutex_;
    mutable BackgroundLoop large_aggregate_bg_worker_;
    mutable BackgroundLoop large_instances_bg_worker_;
    mutable BackgroundLoop small_aggregate_bg_worker_;
    mutable BackgroundLoop small_instances_bg_worker_;
    mutable FastMutex uuid_mutex_;
    size_t uuid_;
    std::atomic_bool shutting_down_;
    ThreadSafeList<std::unique_ptr<const ColorStyle>> color_styles_;
    SceneNodeResources* scene_node_resources_;
    ITrailRenderer* trail_renderer_;
    IDynamicLights* dynamic_lights_;
    mutable std::atomic_uint32_t ncleanups_required_;
    std::list<std::unique_ptr<DanglingBaseClass>> trash_can_obj_;
    std::list<DanglingUniquePtr<SceneNode>> trash_can_child_nodes_;
};

std::ostream& operator << (std::ostream& ostr, const Scene& scene);

}
