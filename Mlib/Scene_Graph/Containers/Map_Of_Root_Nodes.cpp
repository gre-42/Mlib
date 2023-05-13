#include "Map_Of_Root_Nodes.hpp"
#include <Mlib/Memory/Recursive_Deletion.hpp>
#include <Mlib/Scene_Graph/Containers/Root_Nodes.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <ostream>

using namespace Mlib;

MapOfRootNodes::MapOfRootNodes(Scene& scene)
: scene_{ scene }
{}

RootNodes& MapOfRootNodes::create(const std::string& name) {
    auto res = root_nodes_.emplace(name, scene_);
    if (!res.second) {
        THROW_OR_ABORT("Root node with name \"" + name + "\" already exists");
    }
    return res.first->second;
}

bool MapOfRootNodes::root_node_scheduled_for_deletion(const std::string& name, bool must_exist) const {
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

void MapOfRootNodes::clear() {
    clear_container_recursively(root_nodes_);
}

bool MapOfRootNodes::erase(const std::string& name) {
    for (auto& [_, v] : root_nodes_) {
        if (v.erase(name)) {
            return true;
        }
    }
    return false;
}

void MapOfRootNodes::delete_scheduled_root_nodes() const {
    for (const auto& [_, v] : root_nodes_) {
        v.delete_scheduled_root_nodes();
    }
}

bool MapOfRootNodes::no_root_nodes_scheduled_for_deletion() const {
    for (const auto& [_, v] : root_nodes_) {
        if (!v.no_root_nodes_scheduled_for_deletion()) {
            return false;
        }
    }
    return true;
}

void MapOfRootNodes::print(std::ostream& ostr) const {
    for (const auto& [k, v] : root_nodes_) {
        ostr << k << '\n';
        ostr << v;
    }
}

std::ostream& Mlib::operator << (std::ostream& ostr, const MapOfRootNodes& map_of_root_nodes) {
    map_of_root_nodes.print(ostr);
    return ostr;
}
