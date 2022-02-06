#pragma once
#include <Mlib/Regex_Select.hpp>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>

namespace Mlib {

class SceneNode;
class DeleteNodeMutex;
class Scene;

class RootNodes {
    using RootNodesMap = std::map<std::string, std::unique_ptr<SceneNode>>;
public:
    explicit RootNodes(Scene& scene);
    ~RootNodes();
    RootNodesMap::const_iterator find(const std::string& name) const;
    RootNodesMap::const_iterator begin() const;
    RootNodesMap::const_iterator end() const;
    void clear();
    bool erase(const std::string& name);
    bool contains(const std::string& name) const;
    void add_root_node(
        const std::string& name,
        std::unique_ptr<SceneNode>&& scene_node);
    void delete_root_node(const std::string& name);
    void delete_root_nodes(const Mlib::regex& regex);
    bool no_root_nodes_scheduled_for_deletion() const;
    bool root_node_scheduled_for_deletion(const std::string& name) const;
    void schedule_delete_root_node(const std::string& name);
    void delete_scheduled_root_nodes() const;
    void print(std::ostream& ostr) const;
private:
    Scene& scene_;
    RootNodesMap root_nodes_;
    std::set<std::string> root_nodes_to_delete_;
    mutable std::mutex root_nodes_to_delete_mutex_;
};

std::ostream& operator << (std::ostream& ostr, const RootNodes& root_nodes);

}
