#pragma once
#include <cstdint>
#include <map>
#include <string>

namespace Mlib {

template <class T>
class DanglingRef;
class SceneNode;

class ISupplyDepots {
public:
    virtual void add_supply_depot(
        DanglingRef<SceneNode> scene_node,
        const std::map<std::string, uint32_t>& supply_depot,
        float cooldown) = 0;
};

}
