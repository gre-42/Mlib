#pragma once
#include <iosfwd>
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
    void delete_scheduled_root_nodes() const;
    bool no_root_nodes_scheduled_for_deletion() const;
    void shutdown();
    size_t try_empty_the_trash_can();
    void print_trash_can_references() const;
    void print(std::ostream& ostr) const;
private:
    std::map<std::string, RootNodes> root_nodes_;
    Scene& scene_;
};

std::ostream& operator << (std::ostream& ostr, const MapOfRootNodes& map_of_root_nodes);

}
