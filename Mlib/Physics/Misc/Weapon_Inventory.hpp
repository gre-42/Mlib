#pragma once
#include <Mlib/Scene_Graph/Transformation/Node_Modifier.hpp>
#include <functional>
#include <map>
#include <string>

namespace Mlib {

class SceneNode;

class WeaponInventory: public NodeModifier {
public:
    WeaponInventory();
    ~WeaponInventory();
    virtual void modify_node() override;
    void add_weapon(const std::string& weapon_name, const std::function<void()>& create);
    void equip_weapon(const std::string& weapon_name);
    void equip_next_weapon();
    void equip_previous_weapon();
private:
    std::map<std::string, std::function<void()>> create_weapons_;
    std::string equipped_weapon_;
    std::string desired_weapon_;
};

}
