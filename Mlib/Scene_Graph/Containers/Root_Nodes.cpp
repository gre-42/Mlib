#include "Root_Nodes.hpp"
#include <Mlib/Geometry/Intersection/Intersectable_Point.hpp>
#include <Mlib/Memory/Recursive_Deletion.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Threads/Unlock_Guard.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <iostream>

namespace Mlib {

struct RootNodeInfo {
    std::unique_ptr<SceneNode> ptr;
    // ScenePos max_center_distance;
};

}

using namespace Mlib;

RootNodes::RootNodes(Scene& scene)
    : scene_{ scene }
    , invisible_static_nodes_{ "Invisible static nodes" }
    , default_nodes_map_{ "Default nodes" }
    , small_static_nodes_bvh_{ fixed_full<ScenePos, 3>(5), 12 }
    , node_container_{ "Node container" }
    , emptying_trash_can_{ false }
{}

RootNodes::~RootNodes() {
    clear();
    if (!trash_can_.empty()) {
        verbose_abort("RootNodes dtor: Trash-can not empty after clear");
    }
}

RootNodes::DefaultNodesMap& RootNodes::default_nodes() {
    return default_nodes_map_;
}

bool RootNodes::visit_all(const std::function<bool(const DanglingBaseClassRef<const SceneNode>&)>& op) const
{
    for (const auto& [_, node] : default_nodes_map_) {
        if (!op(node)) {
            return false;
        }
    }
    for (const auto& [_, node] : invisible_static_nodes_) {
        if (!op(node)) {
            return false;
        }
    }
    return small_static_nodes_bvh_.visit_all([&op](const auto& d) { return op(d.payload()); });
}

bool RootNodes::visit(
    const FixedArray<ScenePos, 3>& position,
    const std::function<bool(const DanglingBaseClassRef<const SceneNode>&)>& op) const
{
    for (const auto& [_, node] : default_nodes_map_) {
        if (!op(node)) {
            return false;
        }
    }
    return small_static_nodes_bvh_.visit(
        IntersectablePoint{ position },
        op);
}

void RootNodes::clear() {
    scene_.delete_node_mutex_.assert_this_thread_is_deleter_thread();
    small_static_nodes_bvh_.clear();
    invisible_static_nodes_.clear();
    default_nodes_map_.clear();
    clear_map_recursively(
        node_container_,
        [this](const auto& node){
            if (node.mapped().ptr->shutting_down()) {
                verbose_abort("Node \"" + *node.key() + "\" already shutting down");
            }
            node.mapped().ptr->shutdown();
            scene_.unregister_node(node.key());
            root_nodes_to_delete_.erase(node.key());
            trash_can_.push_back(std::move(node.mapped()));
            // linfo() << "add " << node.key();
        });
    if (!root_nodes_to_delete_.empty()) {
        verbose_abort("Root nodes to delete remain after clear");
    }
}

void RootNodes::add_root_node(
    const VariableAndHash<std::string>& name,
    std::unique_ptr<SceneNode>&& scene_node,
    SceneNodeState scene_node_state)
{
    if (root_nodes_to_delete_.contains(name)) {
        THROW_OR_ABORT("Node \"" + *name + "\" is scheduled for deletion");
    }
    if (node_container_.contains(name)) {
        THROW_OR_ABORT("Node \"" + *name + "\" already exists");
    }
    if (scene_node == nullptr) {
        THROW_OR_ABORT("add_root_node received nullptr");
    }
    bool is_static;
    switch (scene_node_state) {
    case SceneNodeState::STATIC:
        is_static = true;
        break;
    case SceneNodeState::DYNAMIC:
        is_static = false;
        break;
    default:
        THROW_OR_ABORT("Unsupported scene node state: " + std::to_string(int(scene_node_state)));
    }
    auto ref = DanglingBaseClassRef<SceneNode>{*scene_node, CURRENT_SOURCE_LOCATION};
    auto md2 = scene_node->max_center_distance2(BILLBOARD_ID_NONE);
    scene_.register_node(name, ref);
    try {
        scene_node->set_scene_and_state(scene_, scene_node_state);
    } catch (const std::runtime_error& e) {
        scene_.unregister_node(name);
        throw;
    }
    if (!node_container_.try_emplace(name, std::move(scene_node)).second) {
        verbose_abort("Could not insert into node container: \"" + *name + '"');
    }
    if (is_static && (md2 != INFINITY)) {
        if (md2 != 0.f) {
            small_static_nodes_bvh_.insert(
                AxisAlignedBoundingBox<ScenePos, 3>::from_center_and_radius(ref->position(), std::sqrt(md2)),
                ref);
        } else if (!invisible_static_nodes_.try_emplace(name, ref).second) {
            verbose_abort("add_root_node could not insert node \"" + *name + '"');
        }
    } else if (!default_nodes_map_.try_emplace(name, ref).second) {
        verbose_abort("add_root_node could not insert node \"" + *name + '"');
    }
}

void RootNodes::move_node_to_bvh(const VariableAndHash<std::string>& name) {
    auto m = invisible_static_nodes_.extract(name);
    if (m.empty()) {
        THROW_OR_ABORT("Could not find non-BVH node with name \"" + *name + '"');
    }
    if (m.mapped()->state() != SceneNodeState::STATIC) {
        invisible_static_nodes_.insert(std::move(m));
        THROW_OR_ABORT("Node \"" + *name + "\" is not static");
    }
    auto md2 = m.mapped()->max_center_distance2(BILLBOARD_ID_NONE);
    if (md2 == 0.f) {
        invisible_static_nodes_.insert(std::move(m));
        THROW_OR_ABORT("Node \"" + *name + "\" has radius=0");
    }
    small_static_nodes_bvh_.insert(
        AxisAlignedBoundingBox<ScenePos, 3>::from_center_and_radius(m.mapped()->position(), std::sqrt(md2)),
        m.mapped());
}

bool RootNodes::contains(const VariableAndHash<std::string>& name) const {
    scene_.delete_node_mutex_.assert_this_thread_is_deleter_thread(); 
    return node_container_.contains(name);
}

std::optional<DanglingBaseClassRef<SceneNode>> RootNodes::try_get(
    const VariableAndHash<std::string>& name,
    SourceLocation loc)
{
    std::shared_lock lock{ scene_.mutex_ };
    auto it = node_container_.try_get(name);
    if (it == nullptr) {
        return std::nullopt;
    }
    return DanglingBaseClassRef<SceneNode>{*it->ptr, loc};
}

bool RootNodes::no_root_nodes_scheduled_for_deletion() const {
    scene_.delete_node_mutex_.assert_this_thread_is_deleter_thread();
    return root_nodes_to_delete_.empty();
}

bool RootNodes::root_node_scheduled_for_deletion(const VariableAndHash<std::string>& name) const {
    std::scoped_lock lock{ root_nodes_to_delete_mutex_ };
    if (!node_container_.contains(name)) {
        THROW_OR_ABORT("No root node with name \"" + *name + "\" exists");
    }
    return root_nodes_to_delete_.contains(name);
}

void RootNodes::schedule_delete_root_node(const VariableAndHash<std::string>& name) {
    scene_.delete_node_mutex_.assert_this_thread_is_deleter_thread();
    std::scoped_lock lock{ root_nodes_to_delete_mutex_ };
    if (!node_container_.contains(name)) {
        verbose_abort("No root node with name \"" + *name + "\" exists");
    }
    if (!root_nodes_to_delete_.insert(name).second) {
        verbose_abort("Node \"" + *name + "\" is already scheduled for deletion");
    }
}

void RootNodes::delete_scheduled_root_nodes() const {
    scene_.delete_node_mutex_.assert_this_thread_is_deleter_thread();
    auto self = const_cast<RootNodes*>(this);
    std::unique_lock lock{ self->root_nodes_to_delete_mutex_ };
    clear_set_recursively(self->root_nodes_to_delete_, [self, &lock](const auto& name){
        UnlockGuard ulock{ lock };
        self->delete_root_node(name);
    });
}

size_t RootNodes::try_empty_the_trash_can() {
    scene_.delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (emptying_trash_can_) {
        THROW_OR_ABORT("Trash can is already being emptied");
    }
    emptying_trash_can_ = true;
    for (auto it = trash_can_.begin(); it != trash_can_.end();) {
        auto c = it++;
        if (c->ptr->nreferences() == 0) {
            trash_can_.erase(c);
        }
    }
    emptying_trash_can_ = false;
    return trash_can_.size();
}

void RootNodes::print_trash_can_references() const {
    for (const auto& node : trash_can_) {
        node.ptr->print_references();
    }
}

void RootNodes::delete_root_node(const VariableAndHash<std::string>& name) {
    scene_.delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (scene_.mutex_.is_owner()) {
        verbose_abort("Node deleter already owns the lock. The UnlockGuard will have no effect");;
    }
    std::unique_lock lock{ scene_.mutex_ };
    auto it = node_container_.try_extract(name);
    if (it.empty()) {
        verbose_abort("RootNodes::delete_root_node: Could not find root node with name \"" + *name + '"');
    }
    if (!it.mapped().ptr->shutting_down()) {
        scene_.unregister_node(name);
        if (default_nodes_map_.erase(name) == 0) {
            small_static_nodes_bvh_.clear();
        }
        root_nodes_to_delete_.erase(name);
        UnlockGuard ulock{ lock };
        it.mapped().ptr->shutdown();
        trash_can_.push_back(std::move(it.mapped()));
        // linfo() << "add " << &trash_can_.back() << " " << it->first;
    }
}

void RootNodes::delete_root_nodes(const Mlib::re::cregex& regex) {
    scene_.delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (scene_.mutex_.is_owner()) {
        verbose_abort("Node deleter already owns the lock. The UnlockGuard will have no effect");;
    }
    std::unique_lock lock{ scene_.mutex_ };
    scene_.unregister_nodes(regex);
    node_container_.erase_if([&](auto& n){
        if (Mlib::re::regex_match(*n.first, regex)) {
            if (default_nodes_map_.erase(n.first) == 0) {
                small_static_nodes_bvh_.clear();
            }
            root_nodes_to_delete_.erase(n.first);
            UnlockGuard ulock{ lock };
            n.second.ptr->shutdown();
            // linfo() << "add " << n.second.ptr.get(DP_LOC).get() << " " << n.first;
            trash_can_.push_back(std::move(n.second));
            return true;
        }
        return false;
    });
}

void RootNodes::print(std::ostream& ostr) const {
    ostr << " Invisible nodes: " << invisible_static_nodes_.size() << '\n';
    ostr << " Default nodes: " << default_nodes_map_.size() << '\n';
    ostr << " Small static nodes: " << small_static_nodes_bvh_.size() << '\n';
    ostr << " Node container\n";
    for (const auto& [k, v] : node_container_) {
        ostr << " " << *k << " #" << v.ptr->nreferences() << '\n';
        v.ptr->print(ostr, 2);
    }
}

std::ostream& Mlib::operator << (std::ostream& ostr, const RootNodes& root_nodes) {
    root_nodes.print(ostr);
    return ostr;
}
