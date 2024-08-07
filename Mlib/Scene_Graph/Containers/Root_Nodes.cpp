#include "Root_Nodes.hpp"
#include <Mlib/Geometry/Intersection/Intersectable_Point.hpp>
#include <Mlib/Memory/Recursive_Deletion.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <iostream>

using namespace Mlib;

RootNodes::RootNodes(Scene& scene)
    : scene_{ scene }
    , small_static_nodes_bvh_{ fixed_full<ScenePos, 3>(5), 12 }
{}

RootNodes::~RootNodes() {
    clear();
}

RootNodes::DefaultNodesMap& RootNodes::default_nodes() {
    return default_nodes_map_;
}

bool RootNodes::visit_all(
    const std::function<bool(const DanglingRef<const SceneNode>&)>& op,
    BvhParallel parallel) const
{
    for (auto& [_, node] : default_nodes_map_) {
        if (!op(node)) {
            return false;
        }
    }
    return small_static_nodes_bvh_.visit_all(
        [&op](const auto& a, const auto& n) { return op(n); },
        parallel);
}

bool RootNodes::visit(
    const FixedArray<ScenePos, 3>& position,
    const std::function<bool(const DanglingRef<const SceneNode>&)>& op,
    BvhParallel parallel) const
{
    for (auto& [_, node] : default_nodes_map_) {
        if (!op(node)) {
            return false;
        }
    }
    return small_static_nodes_bvh_.visit(
        IntersectablePoint{ position },
        op,
        parallel);
}

void RootNodes::clear() {
    small_static_nodes_bvh_.clear();
    default_nodes_map_.clear();
    clear_map_recursively(
        node_container_,
        [this](const auto& node){
            if (node.mapped().ptr->shutting_down()) {
                verbose_abort("Node \"" + node.key() + "\" already shutting down");
            }
            scene_.unregister_node(node.key());
            root_nodes_to_delete_.erase(node.key());
        });
    if (!root_nodes_to_delete_.empty()) {
        verbose_abort("Root nodes to delete remain after clear");
    }
}

void RootNodes::add_root_node(
    const std::string& name,
    DanglingUniquePtr<SceneNode>&& scene_node)
{
    if (root_nodes_to_delete_.contains(name)) {
        THROW_OR_ABORT("Node \"" + name + "\" is scheduled for deletion");
    }
    if (scene_node == nullptr) {
        THROW_OR_ABORT("add_root_node received nullptr");
    }
    bool is_static;
    switch (scene_node->state()) {
    case SceneNodeState::STATIC:
        is_static = true;
        break;
    case SceneNodeState::DYNAMIC:
        is_static = false;
        break;
    default:
        THROW_OR_ABORT("Unsupported scene node state: " + std::to_string(int(scene_node->state())));
    }
    auto ref = scene_node.ref(DP_LOC);
    scene_.register_node(name, ref);
    auto md = scene_node->max_center_distance(UINT32_MAX);
    if (!node_container_.try_emplace(name, std::move(scene_node), md).second) {
        verbose_abort("Could not insert into node container: \"" + name + '"');
    }
    if (is_static && (md != INFINITY)) {
        small_static_nodes_bvh_.insert(
            AxisAlignedBoundingBox<ScenePos, 3>::from_center_and_radius(ref->position(), md),
            ref);
    } else if (!default_nodes_map_.try_emplace(name, ref).second) {
        verbose_abort("add_root_node could not insert node \"" + name + '"');
    }
}

bool RootNodes::erase(const std::string& name) {
    root_nodes_to_delete_.erase(name);
    auto it = node_container_.find(name);
    if (it == node_container_.end()) {
        return false;
    }
    if (default_nodes_map_.erase(name) == 0) {
        small_static_nodes_bvh_.clear();
    }
    node_container_.erase(it);
    return true;
}

bool RootNodes::contains(const std::string& name) const {
    return node_container_.contains(name);
}

std::optional<DanglingRef<SceneNode>> RootNodes::try_get(
    const std::string& name,
    SOURCE_LOCATION loc)
{
    auto it = node_container_.find(name);
    if (it == node_container_.end()) {
        return std::nullopt;
    }
    return it->second.ptr.ref(loc);
}

bool RootNodes::no_root_nodes_scheduled_for_deletion() const {
    return root_nodes_to_delete_.empty();
}

bool RootNodes::root_node_scheduled_for_deletion(const std::string& name) const {
    if (!scene_.delete_node_mutex_.is_locked_by_this_thread() && !scene_.delete_node_mutex_.this_thread_is_deleter_thread()) {
        THROW_OR_ABORT("RootNodes::root_node_scheduled_for_deletion: delete node mutex is not locked by this thread, and this thread is not the deleter thread");
    }
    std::scoped_lock lock{ root_nodes_to_delete_mutex_ };
    if (node_container_.find(name) == node_container_.end()) {
        THROW_OR_ABORT("No root node with name \"" + name + "\" exists");
    }
    return root_nodes_to_delete_.contains(name);
}

void RootNodes::schedule_delete_root_node(const std::string& name) {
    scene_.delete_node_mutex_.assert_this_thread_is_deleter_thread();
    std::scoped_lock lock{ root_nodes_to_delete_mutex_ };
    if (node_container_.find(name) == node_container_.end()) {
        verbose_abort("No root node with name \"" + name + "\" exists");
    }
    if (!root_nodes_to_delete_.insert(name).second) {
        verbose_abort("Node \"" + name + "\" is already scheduled for deletion");
    }
}

void RootNodes::delete_scheduled_root_nodes() const {
    auto self = const_cast<RootNodes*>(this);
    std::unique_lock lock{ self->root_nodes_to_delete_mutex_ };
    clear_set_recursively(self->root_nodes_to_delete_, [self, &lock](const auto& name){
        lock.unlock();
        self->delete_root_node(name);
        lock.lock();
    });
}

void RootNodes::delete_root_node(const std::string& name) {
    scene_.delete_node_mutex_.notify_deleting();
    auto it = node_container_.find(name);
    if (it == node_container_.end()) {
        verbose_abort("RootNodes::delete_root_node: Could not find root node with name \"" + name + '"');
    }
    if (!it->second.ptr->shutting_down()) {
        scene_.unregister_node(name);
        if (default_nodes_map_.erase(name) == 0) {
            small_static_nodes_bvh_.clear();
        }
        root_nodes_to_delete_.erase(name);
        node_container_.erase(it);
    }
}

void RootNodes::delete_root_nodes(const Mlib::regex& regex) {
    scene_.delete_node_mutex_.notify_deleting();
    scene_.unregister_nodes(regex);
    for (auto it = node_container_.begin(); it != node_container_.end(); ) {
        auto n = it++;
        if (Mlib::re::regex_match(n->first, regex)) {
            if (default_nodes_map_.erase(n->first) == 0) {
                small_static_nodes_bvh_.clear();
            }
            root_nodes_to_delete_.erase(n->first);
            node_container_.erase(n->first);
        }
    }
}

void RootNodes::print(std::ostream& ostr) const {
    for (const auto& [k, v] : node_container_) {
        ostr << " " << k << '\n';
        v.ptr->print(ostr, 2);
    }
}

std::ostream& Mlib::operator << (std::ostream& ostr, const RootNodes& root_nodes) {
    root_nodes.print(ostr);
    return ostr;
}
