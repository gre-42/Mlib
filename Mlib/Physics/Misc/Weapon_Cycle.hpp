#pragma once
#include <Mlib/Physics/Misc/Inventory_Item.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/INode_Modifier.hpp>
#include <compare>
#include <functional>
#include <iosfwd>
#include <map>
#include <optional>
#include <string>

namespace Mlib {

class Inventory;
enum class RigidBodyVehicleFlags;
struct BulletProperties;
enum class WhenToEquip;

struct WeaponInfo {
    std::function<void()> create_weapon;
    std::function<void(const VariableAndHash<std::string>& player_name)> create_externals;
    InventoryItem ammo_type;
    const BulletProperties& bullet_properties;
    float cool_down;
    double range_min;
    double range_max;
    float score(double distance_to_target) const;
};

struct PlayerAndWeapon {
    std::optional<VariableAndHash<std::string>> player_name;
    std::string weapon_name;
    std::strong_ordering operator <=> (const PlayerAndWeapon&) const = default;
};

class WeaponCycle: public INodeModifier {
    WeaponCycle(const WeaponCycle&) = delete;
    WeaponCycle& operator = (const WeaponCycle&) = delete;
    friend std::ostream& operator << (std::ostream& ostr, const WeaponCycle& wc);
public:
    WeaponCycle();
    virtual ~WeaponCycle() override;
    virtual void modify_node() override;
    void create_externals(const VariableAndHash<std::string>& player_name);
    void add_weapon(std::string weapon_name, const WeaponInfo& weapon_info);
    void set_desired_weapon(
        std::optional<VariableAndHash<std::string>> player_name,
        std::string weapon_name,
        WhenToEquip when_to_equip);
    void equip_next_weapon(std::optional<VariableAndHash<std::string>> player_name);
    void equip_previous_weapon(std::optional<VariableAndHash<std::string>> player_name);
    InventoryItem ammo_type() const;
    const std::string& weapon_name() const;
    const std::map<std::string, WeaponInfo>& weapon_infos() const;
private:
    std::map<std::string, WeaponInfo> weapon_infos_;
    PlayerAndWeapon equipped_;
    PlayerAndWeapon desired_;
};

std::ostream& operator << (std::ostream& ostr, const WeaponCycle& wc);

}
