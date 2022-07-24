#include "Add_Weapon_To_Cycle.hpp"
#include <Mlib/Macro_Line_Executor.hpp>
#include <Mlib/Physics/Misc/Weapon_Cycle.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(STORAGE_NODE);
DECLARE_OPTION(ENTRY_NAME);
DECLARE_OPTION(AMMO_TYPE);
DECLARE_OPTION(CREATE);

LoadSceneUserFunction AddWeaponToInventory::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*add_weapon_to_cycle"
        "\\s+cycle_node=([\\w+-.]+)"
        "\\s+entry_name=([\\w+-. \\(\\)/]+)"
        "\\s+ammo_type=(\\w+)"
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
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& cycle_node = scene.get_node(match[STORAGE_NODE].str());
    std::string entry_name = match[ENTRY_NAME].str();
    std::string create = match[CREATE].str();
    WeaponCycle* wc = dynamic_cast<WeaponCycle*>(&cycle_node.get_node_modifier());
    if (wc == nullptr) {
        throw std::runtime_error("Node modifier is not a weapon inventory");
    }
    std::string ammo_type = match[AMMO_TYPE].str();
    wc->add_weapon(
        entry_name,
        WeaponInfo{
            .create_weapon = [macro_line_executor = args.macro_line_executor, create, ammo_type, &rsc = args.rsc](){
                SubstitutionMap subst;
                subst.insert("AMMO_TYPE", ammo_type);
                macro_line_executor(create, &subst, rsc);
            },
            .ammo_type = ammo_type});
}
