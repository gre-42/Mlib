#include "Root_Nodes.hpp"
#include <Mlib/Recursive_Deletion.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
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

void RootNodes::clear() {
    clear_map_recursively(
        root_nodes_,
        [this](const auto& node){
            scene_.unregister_node(node.key());
            root_nodes_to_delete_.erase(node.key());
        });
    if (!root_nodes_to_delete_.empty()) {
        throw std::runtime_error("Root nodes to delete remain after clear");
    }
}

void RootNodes::add_root_node(
    const std::string& name,
    std::unique_ptr<SceneNode>&& scene_node)
{
    if (root_nodes_to_delete_.contains(name)) {
        throw std::runtime_error("Node \"" + name + "\" is scheduled for deletion");
    }
    scene_.register_node(name, scene_node.get());
    if (!root_nodes_.insert({ name, std::move(scene_node) }).second) {
        throw std::runtime_error("add_root_node could not insert node");
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
        throw std::runtime_error("RootNodes::root_node_scheduled_for_deletion: delete node mutex is not locked, and this thread is not the deleter thread");
    }
    std::lock_guard lock{ root_nodes_to_delete_mutex_ };
    if (root_nodes_.find(name) == root_nodes_.end()) {
        throw std::runtime_error("No root node with name \"" + name + "\" exists");
    }
    return root_nodes_to_delete_.contains(name);
}

void RootNodes::schedule_delete_root_node(const std::string& name) {
    scene_.delete_node_mutex_.assert_this_thread_is_deleter_thread();
    std::lock_guard lock{ root_nodes_to_delete_mutex_ };
    if (root_nodes_.find(name) == root_nodes_.end()) {
        throw std::runtime_error("No root node with name \"" + name + "\" exists");
    }
    if (!root_nodes_to_delete_.insert(name).second) {
        throw std::runtime_error("Node \"" + name + "\" is already scheduled for deletion");
    }
}

void RootNodes::delete_scheduled_root_nodes() const {
    auto self = const_cast<RootNodes*>(this);
    std::lock_guard lock{ self->root_nodes_to_delete_mutex_ };
    clear_set_recursively(self->root_nodes_to_delete_, [self](const auto& name){
        self->delete_root_node(name);
    });
}

void RootNodes::delete_root_node(const std::string& name) {
    std::cerr << "Thread " << std::this_thread::get_id() << ": RootNodes::delete_root_node (0) \"" << name << '"' << std::endl;
    scene_.delete_node_mutex_.notify_deleting();
    std::cerr << "Thread " << std::this_thread::get_id() << ": RootNodes::delete_root_node (1) \"" << name << '"' << std::endl;
    auto it = root_nodes_.find(name);
    if (it == root_nodes_.end()) {
        throw std::runtime_error("RootNodes::delete_root_node: Could not find root node with name \"" + name + '"');
    }
    if (!it->second->shutting_down()) {
        std::cerr << "Thread " << std::this_thread::get_id() << ": RootNodes::delete_root_node (2) \"" << name << '"' << std::endl;
        scene_.unregister_node(name);
        std::cerr << "Thread " << std::this_thread::get_id() << ": RootNodes::delete_root_node (3) \"" << name << '"' << std::endl;
        root_nodes_to_delete_.erase(name);
        std::cerr << "Thread " << std::this_thread::get_id() << ": RootNodes::delete_root_node (4) \"" << name << '"' << std::endl;
        root_nodes_.erase(it);
        std::cerr << "Thread " << std::this_thread::get_id() << ": RootNodes::delete_root_node (5)" << std::endl;
    }
}

void RootNodes::delete_root_nodes(const Mlib::regex& regex) {
    scene_.delete_node_mutex_.notify_deleting();
    for (auto it = root_nodes_.begin(); it != root_nodes_.end(); ) {
        auto n = it++;
        if (Mlib::re::regex_match(n->first, regex)) {
            root_nodes_.erase(n->first);
            root_nodes_to_delete_.erase(n->first);
        }
    }
    scene_.unregister_nodes(regex);
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
