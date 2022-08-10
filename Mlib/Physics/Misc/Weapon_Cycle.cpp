#include "Weapon_Cycle.hpp"
#include <Mlib/Physics/Misc/Inventory.hpp>
#include <stdexcept>

using namespace Mlib;

WeaponCycle::WeaponCycle(Inventory& inventory)
: inventory_{inventory}
{}

WeaponCycle::~WeaponCycle()
{}

void WeaponCycle::modify_node() {
    if (equipped_weapon_ != desired_weapon_) {
        auto it = weapon_infos_.find(desired_weapon_);
        if (it == weapon_infos_.end()) {
            throw std::runtime_error("Inventory does not have information about a weapon with name \"" + desired_weapon_ + '"');
        }
        it->second.create_weapon();
        equipped_weapon_ = desired_weapon_;
    }
}

void WeaponCycle::add_weapon(const std::string& weapon_name, const WeaponInfo& weapon_info)
{
    if (!weapon_infos_.insert({ weapon_name, weapon_info }).second)  {
        throw std::runtime_error("Inventory already has information about a weapon with name \"" + weapon_name + '"');
    }
}

void WeaponCycle::set_desired_weapon(const std::string& weapon_name) {
    desired_weapon_ = weapon_name;
}

void WeaponCycle::equip_next_weapon() {
    if (weapon_infos_.empty()) {
        return;
    }
    auto it = weapon_infos_.find(desired_weapon_);
    if (it == weapon_infos_.end()) {
        desired_weapon_ = weapon_infos_.begin()->first;
    } else {
        ++it;
        if (it == weapon_infos_.end()) {
            return;
        }
        desired_weapon_ = it->first;
    }
}

void WeaponCycle::equip_previous_weapon() {
    if (weapon_infos_.empty()) {
        return;
    }
    auto it = weapon_infos_.find(desired_weapon_);
    if (it == weapon_infos_.end()) {
        desired_weapon_ = weapon_infos_.rbegin()->first;
    } else if (it != weapon_infos_.begin()) {
        --it;
        desired_weapon_ = it->first;
    }
}

std::string WeaponCycle::ammo_type() const {
    auto it = weapon_infos_.find(equipped_weapon_);
    if (it == weapon_infos_.end()) {
        throw std::runtime_error("Inventory does not have information about a weapon with name \"" + equipped_weapon_ + '"');
    }
    return it->second.ammo_type;
}
