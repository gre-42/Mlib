#pragma once
#include <Mlib/Physics/Misc/Inventory_Item.hpp>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace Mlib {

template <class T>
class DanglingBaseClassRef;
class SceneNode;

class ISupplyDepots {
public:
    virtual void add_supply_depot(
        const DanglingBaseClassRef<SceneNode>& scene_node,
        const std::unordered_map<InventoryItem, uint32_t>& supply_depot,
        float cooldown) = 0;
};

}
