#include "Equip_Weapon.hpp"
#include <Mlib/Physics/Misc/Weapon_Inventory.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(STORAGE_NODE);
DECLARE_OPTION(ENTRY_NAME);

LoadSceneUserFunction EquipWeapon::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*equip_weapon"
        "\\s+storage_node=([\\w+-.]+)"
        "\\s+entry_name=([\\w-. \\(\\)/+-]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        EquipWeapon(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

EquipWeapon::EquipWeapon(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void EquipWeapon::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& storage_node = scene.get_node(match[STORAGE_NODE].str());
    std::string entry_name = match[ENTRY_NAME].str();
    WeaponInventory* wi = dynamic_cast<WeaponInventory*>(storage_node.get_node_modifier());
    if (wi == nullptr) {
        throw std::runtime_error("Node modifier is not a weapon inventory");
    }
    wi->equip_weapon(entry_name);
}
