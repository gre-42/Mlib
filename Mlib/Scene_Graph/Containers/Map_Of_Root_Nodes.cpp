
#include "Map_Of_Root_Nodes.hpp"
#include <Mlib/Memory/Recursive_Deletion.hpp>
#include <Mlib/Scene_Graph/Containers/Root_Nodes.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Threads/Throwing_Lock_Guard.hpp>
#include <ostream>
#include <stdexcept>

using namespace Mlib;

MapOfRootNodes::MapOfRootNodes(const DanglingBaseClassRef<Scene>& scene)
    : root_nodes_{ "Root node" }
    , scene_{ scene }
{}

RootNodes& MapOfRootNodes::create(VariableAndHash<std::string> name) {
    ThrowingLockGuard delete_lock{scene_->delete_node_mutex};
    return root_nodes_.add(std::move(name), scene_);
}

void MapOfRootNodes::shutdown() {
    ThrowingLockGuard delete_lock{scene_->delete_node_mutex};
    for (auto& [_, v] : root_nodes_) {
        v.clear();
    }
}

void MapOfRootNodes::print_trash_can_references() const {
    ThrowingLockGuard delete_lock{scene_->delete_node_mutex};
    for (auto& [_, v] : root_nodes_) {
        v.print_trash_can_references();
    }
}

size_t MapOfRootNodes::try_empty_the_trash_can() {
    ThrowingLockGuard delete_lock{scene_->delete_node_mutex};
    size_t nremaining = 0;
    for (auto& [_, v] : root_nodes_) {
        nremaining += v.try_empty_the_trash_can();
    }
    return nremaining;
}

void MapOfRootNodes::print(std::ostream& ostr) const {
    ThrowingLockGuard delete_lock{scene_->delete_node_mutex};
    for (const auto& [k, v] : root_nodes_) {
        ostr << *k << '\n';
        ostr << v;
    }
}

std::ostream& Mlib::operator << (std::ostream& ostr, const MapOfRootNodes& map_of_root_nodes) {
    map_of_root_nodes.print(ostr);
    return ostr;
}
