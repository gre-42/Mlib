#pragma once
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/INode_Modifier.hpp>
#include <compare>
#include <functional>
#include <map>
#include <optional>
#include <string>

namespace Mlib {

class Inventory;
enum class RigidBodyVehicleFlags;
struct BulletProperties;

struct WeaponInfo {
    std::function<void(const std::optional<std::string>& player_name)> create_weapon;
    std::string ammo_type;
    const BulletProperties& bullet_properties;
    float cool_down;
    double range_min;
    double range_max;
    float score(double distance_to_target) const;
};

struct PlayerAndWeapon {
    std::optional<std::string> player_name;
    std::string weapon_name;
    std::strong_ordering operator <=> (const PlayerAndWeapon&) const = default;
};

class WeaponCycle: public INodeModifier {
    WeaponCycle(const WeaponCycle&) = delete;
    WeaponCycle& operator = (const WeaponCycle&) = delete;
public:
    WeaponCycle();
    virtual ~WeaponCycle() override;
    virtual void modify_node() override;
    void add_weapon(std::string weapon_name, const WeaponInfo& weapon_info);
    void set_desired_weapon(
        std::optional<std::string> player_name,
        std::string weapon_name);
    void equip_next_weapon(std::optional<std::string> player_name);
    void equip_previous_weapon(std::optional<std::string> player_name);
    std::string ammo_type() const;
    const std::map<std::string, WeaponInfo>& weapon_infos() const;
private:
    std::map<std::string, WeaponInfo> weapon_infos_;
    PlayerAndWeapon equipped_;
    PlayerAndWeapon desired_;
};

}
