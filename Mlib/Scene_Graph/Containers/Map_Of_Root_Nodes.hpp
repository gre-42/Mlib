#pragma once
#include <Mlib/Map/String_With_Hash_Unordered_Map.hpp>
#include <iosfwd>
#include <string>

namespace Mlib {

class RootNodes;
class Scene;

class MapOfRootNodes {
public:
    explicit MapOfRootNodes(Scene& scene);
    RootNodes& create(VariableAndHash<std::string> name);
    void shutdown();
    size_t try_empty_the_trash_can();
    void print_trash_can_references() const;
    void print(std::ostream& ostr) const;
private:
    StringWithHashUnorderedMap<RootNodes> root_nodes_;
    Scene& scene_;
};

std::ostream& operator << (std::ostream& ostr, const MapOfRootNodes& map_of_root_nodes);

}
