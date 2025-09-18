#include "Weapon_Cycle.hpp"
#include <Mlib/Physics/Bullets/Bullet_Properties.hpp>
#include <Mlib/Physics/Misc/Inventory.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

float WeaponInfo::score(double distance_to_target) const {
    if (cool_down == 0) {
        THROW_OR_ABORT("Weapon has cooldown 0");
    }
    if ((distance_to_target < range_min) ||
        (distance_to_target > range_max))
    {
        return -INFINITY;
    }
    return bullet_properties.damage / (cool_down / seconds);
}

WeaponCycle::WeaponCycle() = default;

WeaponCycle::~WeaponCycle() = default;

void WeaponCycle::modify_node() {
    if (equipped_ != desired_) {
        auto it = weapon_infos_.find(desired_.weapon_name);
        if (it == weapon_infos_.end()) {
            THROW_OR_ABORT("Inventory does not have information about a weapon with name \"" + desired_.weapon_name + '"');
        }
        if (it->second.create_weapon) {
            it->second.create_weapon();
        }
        if (it->second.create_externals && desired_.player_name.has_value()) {
            it->second.create_externals(*desired_.player_name);
        }
        equipped_ = desired_;
    }
}

void WeaponCycle::create_externals(const std::string& player_name) {
    auto it = weapon_infos_.find(equipped_.weapon_name);
    if (it == weapon_infos_.end()) {
        THROW_OR_ABORT("Inventory does not have information about a weapon with name \"" + equipped_.weapon_name + '"');
    }
    if (it->second.create_externals) {
        it->second.create_externals(player_name);
    }
}

void WeaponCycle::add_weapon(std::string weapon_name, const WeaponInfo& weapon_info)
{
    if (!weapon_infos_.try_emplace(std::move(weapon_name), weapon_info).second)  {
        THROW_OR_ABORT("Inventory already has information about a weapon with name \"" + weapon_name + '"');
    }
}

void WeaponCycle::set_desired_weapon(
    std::optional<std::string> player_name,
    std::string weapon_name)
{
    desired_ = { std::move(player_name), std::move(weapon_name) };
}

void WeaponCycle::equip_next_weapon(std::optional<std::string> player_name) {
    if (weapon_infos_.empty()) {
        return;
    }
    auto it = weapon_infos_.find(desired_.weapon_name);
    if (it == weapon_infos_.end()) {
        desired_.weapon_name = weapon_infos_.begin()->first;
    } else {
        ++it;
        if (it == weapon_infos_.end()) {
            return;
        }
        desired_.weapon_name = it->first;
    }
    desired_.player_name = std::move(player_name);
}

void WeaponCycle::equip_previous_weapon(std::optional<std::string> player_name) {
    if (weapon_infos_.empty()) {
        return;
    }
    auto it = weapon_infos_.find(desired_.weapon_name);
    if (it == weapon_infos_.end()) {
        desired_.weapon_name = weapon_infos_.rbegin()->first;
    } else if (it != weapon_infos_.begin()) {
        --it;
        desired_.weapon_name = it->first;
    }
    desired_.player_name = std::move(player_name);
}

std::string WeaponCycle::ammo_type() const {
    auto it = weapon_infos_.find(equipped_.weapon_name);
    if (it == weapon_infos_.end()) {
        THROW_OR_ABORT("Inventory does not have information about a weapon with name \"" + equipped_.weapon_name + '"');
    }
    return it->second.ammo_type;
}

const std::map<std::string, WeaponInfo>& WeaponCycle::weapon_infos() const {
    return weapon_infos_;
}
