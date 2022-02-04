#include "Add_Weapon_To_Inventory.hpp"
#include <Mlib/Macro_Line_Executor.hpp>
#include <Mlib/Physics/Misc/Weapon_Inventory.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(STORAGE_NODE);
DECLARE_OPTION(ENTRY_NAME);
DECLARE_OPTION(CREATE);

LoadSceneUserFunction AddWeaponToInventory::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*add_weapon_to_inventory"
        "\\s+storage_node=([\\w+-.]+)"
        "\\s+entry_name=([\\w-. \\(\\)/+-]+)"
        "\\s+create=([\\s\\S]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        AddWeaponToInventory(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

AddWeaponToInventory::AddWeaponToInventory(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void AddWeaponToInventory::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto storage_node = scene.get_node(match[STORAGE_NODE].str());
    std::string entry_name = match[ENTRY_NAME].str();
    std::string create = match[CREATE].str();
    WeaponInventory* wi = dynamic_cast<WeaponInventory*>(storage_node->get_node_modifier());
    if (wi == nullptr) {
        throw std::runtime_error("Node modifier is not a weapon inventory");
    }
    wi->add_weapon(entry_name, [macro_line_executor = args.macro_line_executor, create, &rsc = args.rsc](){
        macro_line_executor(create, nullptr, rsc);
    });
}
