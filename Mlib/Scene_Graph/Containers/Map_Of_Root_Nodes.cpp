#include "Map_Of_Root_Nodes.hpp"
#include <Mlib/Memory/Recursive_Deletion.hpp>
#include <Mlib/Scene_Graph/Containers/Root_Nodes.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <ostream>

using namespace Mlib;

MapOfRootNodes::MapOfRootNodes(Scene& scene)
    : scene_{ scene }
{}

RootNodes& MapOfRootNodes::create(const std::string& name) {
    scene_.delete_node_mutex().assert_this_thread_is_deleter_thread();
    auto res = root_nodes_.emplace(name, scene_);
    if (!res.second) {
        THROW_OR_ABORT("Root node with name \"" + name + "\" already exists");
    }
    return res.first->second;
}

bool MapOfRootNodes::root_node_scheduled_for_deletion(const std::string& name, bool must_exist) const {
    scene_.delete_node_mutex().assert_this_thread_is_deleter_thread();
    for (const auto& [_, v] : root_nodes_) {
        if (v.contains(name)) {
            return v.root_node_scheduled_for_deletion(name);
        }
    }
    if (must_exist) {
        THROW_OR_ABORT("MapOfRootNodes::root_node_scheduled_for_deletion: Could not find root node with name \"" + name + '"');
    } else {
        return false;
    }
}

void MapOfRootNodes::shutdown() {
    scene_.delete_node_mutex().assert_this_thread_is_deleter_thread();
    for (auto& [_, v] : root_nodes_) {
        v.clear();
    }
}

void MapOfRootNodes::print_trash_can_references() const {
    scene_.delete_node_mutex().assert_this_thread_is_deleter_thread();
    for (auto& [_, v] : root_nodes_) {
        v.print_trash_can_references();
    }
}

void MapOfRootNodes::delete_scheduled_root_nodes() const {
    scene_.delete_node_mutex().assert_this_thread_is_deleter_thread();
    for (const auto& [_, v] : root_nodes_) {
        v.delete_scheduled_root_nodes();
    }
}

bool MapOfRootNodes::no_root_nodes_scheduled_for_deletion() const {
    scene_.delete_node_mutex().assert_this_thread_is_deleter_thread();
    for (const auto& [_, v] : root_nodes_) {
        if (!v.no_root_nodes_scheduled_for_deletion()) {
            return false;
        }
    }
    return true;
}

size_t MapOfRootNodes::try_empty_the_trash_can() {
    scene_.delete_node_mutex().assert_this_thread_is_deleter_thread();
    size_t nremaining = 0;
    for (auto& [_, v] : root_nodes_) {
        nremaining += v.try_empty_the_trash_can();
    }
    return nremaining;
}

void MapOfRootNodes::print(std::ostream& ostr) const {
    scene_.delete_node_mutex().assert_this_thread_is_deleter_thread();
    for (const auto& [k, v] : root_nodes_) {
        ostr << k << '\n';
        ostr << v;
    }
}

std::ostream& Mlib::operator << (std::ostream& ostr, const MapOfRootNodes& map_of_root_nodes) {
    map_of_root_nodes.print(ostr);
    return ostr;
}
