#include "Create_Weapon_Inventory.hpp"
#include <Mlib/Physics/Misc/Weapon_Inventory.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(STORAGE_NODE);

LoadSceneUserFunction CreateWeaponInventory::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*create_weapon_inventory"
        "\\s+storage_node=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateWeaponInventory(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateWeaponInventory::CreateWeaponInventory(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateWeaponInventory::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& storage_node = scene.get_node(match[STORAGE_NODE].str());
    storage_node.set_node_modifier(std::make_unique<WeaponInventory>());
}
