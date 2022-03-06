#pragma once
#include <map>
#include <string>

namespace Mlib {

class RootNodes;
class Scene;

class MapOfRootNodes {
public:
    explicit MapOfRootNodes(Scene& scene);
    RootNodes& create(const std::string& name);
    bool root_node_scheduled_for_deletion(
        const std::string& name,
        bool must_exist = true) const;
    void clear();
    bool erase(const std::string& name);
    void delete_scheduled_root_nodes() const;
    bool no_root_nodes_scheduled_for_deletion() const;
    void print(std::ostream& ostr) const;
private:
    std::map<std::string, RootNodes> root_nodes_;
    Scene& scene_;
};

std::ostream& operator << (std::ostream& ostr, const MapOfRootNodes& map_of_root_nodes);

}
