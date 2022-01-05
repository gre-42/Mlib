#include "Weapon_Inventory.hpp"
#include <stdexcept>

using namespace Mlib;

WeaponInventory::WeaponInventory(SceneNode& weapon_visual_node, SceneNode& weapon_physics_node)
: weapon_visual_node_{ weapon_visual_node },
  weapon_physics_node_{ weapon_physics_node }
{}

void WeaponInventory::modify_node() {
    if (equipped_weapon_ != desired_weapon_) {
        auto it = create_weapons_.find(desired_weapon_);
        if (it == create_weapons_.end()) {
            throw std::runtime_error("Inventory does not contain weapon with name \"" + desired_weapon_ + '"');
        }
        it->second();
        equipped_weapon_ = desired_weapon_;
    }
}

void WeaponInventory::add_weapon(const std::string& weapon_name, const std::function<void()>& create) {
    if (!create_weapons_.insert({ weapon_name, create }).second)  {
        throw std::runtime_error("Inventory already contains weapon with name \"" + weapon_name + '"');
    }
}

void WeaponInventory::equip_weapon(const std::string& weapon_name) {
    desired_weapon_ = weapon_name;
}
