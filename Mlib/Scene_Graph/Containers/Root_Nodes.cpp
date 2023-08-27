#include "Root_Nodes.hpp"
#include <Mlib/Memory/Recursive_Deletion.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <iostream>

using namespace Mlib;

RootNodes::RootNodes(Scene& scene)
: scene_{ scene }
{}

RootNodes::~RootNodes() {
    clear();
}

RootNodes::RootNodesMap::const_iterator RootNodes::find(const std::string& name) const {
    return root_nodes_.find(name);
}

RootNodes::RootNodesMap::const_iterator RootNodes::begin() const {
    return root_nodes_.begin();
}

RootNodes::RootNodesMap::const_iterator RootNodes::end() const {
    return root_nodes_.end();
}

RootNodes::RootNodesMap::iterator RootNodes::begin() {
    return root_nodes_.begin();
}

RootNodes::RootNodesMap::iterator RootNodes::end() {
    return root_nodes_.end();
}

void RootNodes::clear() {
    clear_map_recursively(
        root_nodes_,
        [this](const auto& node){
            if (node.mapped()->shutting_down()) {
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
    scene_.register_node(name, scene_node.ref(DP_LOC));
    if (!root_nodes_.insert({ name, std::move(scene_node) }).second) {
        THROW_OR_ABORT("add_root_node could not insert node");
    };
}

bool RootNodes::erase(const std::string& name) {
    root_nodes_to_delete_.erase(name);
    return (root_nodes_.erase(name) == 1);
}

bool RootNodes::contains(const std::string& name) const {
    return root_nodes_.contains(name);
}

bool RootNodes::no_root_nodes_scheduled_for_deletion() const {
    return root_nodes_to_delete_.empty();
}

bool RootNodes::root_node_scheduled_for_deletion(const std::string& name) const {
    if (!scene_.delete_node_mutex_.is_locked_by_this_thread() && !scene_.delete_node_mutex_.this_thread_is_deleter_thread()) {
        THROW_OR_ABORT("RootNodes::root_node_scheduled_for_deletion: delete node mutex is not locked by this thread, and this thread is not the deleter thread");
    }
    std::scoped_lock lock{ root_nodes_to_delete_mutex_ };
    if (root_nodes_.find(name) == root_nodes_.end()) {
        THROW_OR_ABORT("No root node with name \"" + name + "\" exists");
    }
    return root_nodes_to_delete_.contains(name);
}

void RootNodes::schedule_delete_root_node(const std::string& name) {
    scene_.delete_node_mutex_.assert_this_thread_is_deleter_thread();
    std::scoped_lock lock{ root_nodes_to_delete_mutex_ };
    if (root_nodes_.find(name) == root_nodes_.end()) {
        verbose_abort("No root node with name \"" + name + "\" exists");
    }
    if (!root_nodes_to_delete_.insert(name).second) {
        verbose_abort("Node \"" + name + "\" is already scheduled for deletion");
    }
}

void RootNodes::delete_scheduled_root_nodes() const {
    auto self = const_cast<RootNodes*>(this);
    std::scoped_lock lock{ self->root_nodes_to_delete_mutex_ };
    clear_set_recursively(self->root_nodes_to_delete_, [self](const auto& name){
        self->delete_root_node(name);
    });
}

void RootNodes::delete_root_node(const std::string& name) {
    scene_.delete_node_mutex_.notify_deleting();
    auto it = root_nodes_.find(name);
    if (it == root_nodes_.end()) {
        verbose_abort("RootNodes::delete_root_node: Could not find root node with name \"" + name + '"');
    }
    if (!it->second->shutting_down()) {
        scene_.unregister_node(name);
        root_nodes_to_delete_.erase(name);
        root_nodes_.erase(it);
    }
}

void RootNodes::delete_root_nodes(const Mlib::regex& regex) {
    scene_.delete_node_mutex_.notify_deleting();
    scene_.unregister_nodes(regex);
    for (auto it = root_nodes_.begin(); it != root_nodes_.end(); ) {
        auto n = it++;
        if (Mlib::re::regex_match(n->first, regex)) {
            root_nodes_to_delete_.erase(n->first);
            root_nodes_.erase(n->first);
        }
    }
}

void RootNodes::print(std::ostream& ostr) const {
    for (const auto& n : root_nodes_) {
        ostr << " " << n.first << '\n';
        n.second->print(ostr, 2);
    }
}

std::ostream& Mlib::operator << (std::ostream& ostr, const RootNodes& root_nodes) {
    root_nodes.print(ostr);
    return ostr;
}
