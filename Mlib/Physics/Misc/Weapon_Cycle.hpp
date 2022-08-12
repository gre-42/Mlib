#pragma once
#include <Mlib/Scene_Graph/Transformation/Node_Modifier.hpp>
#include <functional>
#include <map>
#include <string>

namespace Mlib {

class Inventory;

struct WeaponInfo {
    std::function<void()> create_weapon;
    std::string ammo_type;
    float cool_down;
    float bullet_damage;
    float score() const;
};

class WeaponCycle: public NodeModifier {
public:
    explicit WeaponCycle(Inventory& inventory);
    ~WeaponCycle();
    virtual void modify_node() override;
    void add_weapon(const std::string& weapon_name, const WeaponInfo& weapon_info);
    void set_desired_weapon(const std::string& weapon_name);
    void equip_next_weapon();
    void equip_previous_weapon();
    std::string ammo_type() const;
    const std::map<std::string, WeaponInfo>& weapon_infos() const;
private:
    Inventory& inventory_;
    std::map<std::string, WeaponInfo> weapon_infos_;
    std::string equipped_weapon_;
    std::string desired_weapon_;
};

}
