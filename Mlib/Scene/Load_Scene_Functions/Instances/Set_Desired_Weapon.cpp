#include "Set_Desired_Weapon.hpp"
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

LoadSceneUserFunction SetDesiredWeapon::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_desired_weapon"
        "\\s+cycle_node=([\\w+-.]+)"
        "\\s+entry_name=([\\w+-. \\(\\)/]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        SetDesiredWeapon(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

SetDesiredWeapon::SetDesiredWeapon(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetDesiredWeapon::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& cycle_node = scene.get_node(match[STORAGE_NODE].str());
    std::string entry_name = match[ENTRY_NAME].str();
    WeaponCycle* wi = dynamic_cast<WeaponCycle*>(&cycle_node.get_node_modifier());
    if (wi == nullptr) {
        throw std::runtime_error("Node modifier is not a weapon inventory");
    }
    wi->set_desired_weapon(entry_name);
}
