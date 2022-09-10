#pragma once
#include <cstdint>
#include <map>
#include <string>

namespace Mlib {

class SceneNode;

class ISupplyDepots {
public:
    virtual void add_supply_depot(
        SceneNode& scene_node,
        const std::map<std::string, uint32_t>& supply_depot,
        float cooldown) = 0;
};

}
