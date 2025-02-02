#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Threads/Atomic_Mutex.hpp>
#include <functional>
#include <map>
#include <set>
#include <string>

namespace Mlib {

class SceneNode;
class DeleteNodeMutex;
class Scene;
struct RootNodeInfo;
enum class SceneNodeState;

class RootNodes {
    using DefaultNodesMap = std::map<std::string, DanglingRef<SceneNode>>;
    using SmallStaticNodesBvh = Bvh<ScenePos, 3, DanglingRef<SceneNode>>;
    using NodeContainer = std::map<std::string, RootNodeInfo>;
    using TrashCan = std::list<RootNodeInfo>;
    RootNodes(const RootNodes&) = delete;
    RootNodes& operator = (const RootNodes&) = delete;
public:
    explicit RootNodes(Scene& scene);
    ~RootNodes();
    DefaultNodesMap& default_nodes();
    bool visit_all(const std::function<bool(const DanglingRef<const SceneNode>&)>& op) const;
    bool visit(
        const FixedArray<ScenePos, 3>& position,
        const std::function<bool(const DanglingRef<const SceneNode>&)>& op) const;
    void clear();
    bool contains(const std::string& name) const;
    std::optional<DanglingRef<SceneNode>> try_get(
        const std::string& name,
        SOURCE_LOCATION loc);
    void add_root_node(
        const std::string& name,
        DanglingUniquePtr<SceneNode>&& scene_node,
        SceneNodeState scene_node_state);
    void move_node_to_bvh(const std::string& name);
    void delete_root_node(const std::string& name);
    void delete_root_nodes(const Mlib::regex& regex);
    bool no_root_nodes_scheduled_for_deletion() const;
    bool root_node_scheduled_for_deletion(const std::string& name) const;
    void schedule_delete_root_node(const std::string& name);
    void delete_scheduled_root_nodes() const;
    size_t try_empty_the_trash_can();
    void print_trash_can_references() const;
    void print(std::ostream& ostr) const;
private:
    Scene& scene_;
    DefaultNodesMap nodes_under_construction_;
    DefaultNodesMap default_nodes_map_;             // Contains nodes that are large or moving
    SmallStaticNodesBvh small_static_nodes_bvh_;    // Contains nodes that are small and static
    NodeContainer node_container_;
    TrashCan trash_can_;
    std::set<std::string> root_nodes_to_delete_;
    mutable AtomicMutex root_nodes_to_delete_mutex_;
    bool emptying_trash_can_;
};

std::ostream& operator << (std::ostream& ostr, const RootNodes& root_nodes);

}
