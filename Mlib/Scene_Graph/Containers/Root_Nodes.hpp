#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Map/String_With_Hash_Unordered_Map.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <functional>
#include <string>

namespace Mlib {

class SceneNode;
class DeleteNodeMutex;
class Scene;
struct RootNodeInfo;
enum class SceneNodeState;

class RootNodes {
    using DefaultNodesMap = StringWithHashUnorderedMap<DanglingBaseClassRef<SceneNode>>;
    using SmallStaticNodesBvh = Bvh<ScenePos, 3, DanglingBaseClassRef<SceneNode>>;
    using NodeContainer = StringWithHashUnorderedMap<RootNodeInfo>;
    using TrashCan = std::list<RootNodeInfo>;
    RootNodes(const RootNodes&) = delete;
    RootNodes& operator = (const RootNodes&) = delete;
public:
    explicit RootNodes(Scene& scene);
    ~RootNodes();
    DefaultNodesMap& default_nodes();
    bool visit_all(const std::function<bool(const DanglingBaseClassRef<const SceneNode>&)>& op) const;
    bool visit(
        const FixedArray<ScenePos, 3>& position,
        const std::function<bool(const DanglingBaseClassRef<const SceneNode>&)>& op) const;
    void clear();
    bool contains(const VariableAndHash<std::string>& name) const;
    std::optional<DanglingBaseClassRef<SceneNode>> try_get(
        const VariableAndHash<std::string>& name,
        SourceLocation loc);
    void add_root_node(
        const VariableAndHash<std::string>& name,
        std::unique_ptr<SceneNode>&& scene_node,
        SceneNodeState scene_node_state);
    void move_node_to_bvh(const VariableAndHash<std::string>& name);
    void delete_root_node(const VariableAndHash<std::string>& name);
    void delete_root_nodes(const Mlib::re::cregex& regex);
    size_t try_empty_the_trash_can();
    void print_trash_can_references() const;
    void print(std::ostream& ostr) const;
private:
    Scene& scene_;
    DefaultNodesMap invisible_static_nodes_;
    DefaultNodesMap default_nodes_map_;             // Contains nodes that are large or moving
    SmallStaticNodesBvh small_static_nodes_bvh_;    // Contains nodes that are small and static
    NodeContainer node_container_;
    TrashCan trash_can_;
    bool emptying_trash_can_;
};

std::ostream& operator << (std::ostream& ostr, const RootNodes& root_nodes);

}
