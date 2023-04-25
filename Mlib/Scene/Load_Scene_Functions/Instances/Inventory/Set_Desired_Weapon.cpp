#include "Set_Desired_Weapon.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Misc/Weapon_Cycle.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(cycle_node);
DECLARE_ARGUMENT(entry_name);
DECLARE_ARGUMENT(equip_instantly);
}

const std::string SetDesiredWeapon::key = "set_desired_weapon";

LoadSceneJsonUserFunction SetDesiredWeapon::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SetDesiredWeapon(args.renderable_scene()).execute(args);
};

SetDesiredWeapon::SetDesiredWeapon(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetDesiredWeapon::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto& cycle_node = scene.get_node(args.arguments.at<std::string>(KnownArgs::cycle_node));
    std::string entry_name = args.arguments.at<std::string>(KnownArgs::entry_name);
    WeaponCycle* wi = dynamic_cast<WeaponCycle*>(&cycle_node.get_node_modifier());
    if (wi == nullptr) {
        THROW_OR_ABORT("Node modifier is not a weapon inventory");
    }
    wi->set_desired_weapon(entry_name);
    if (args.arguments.at<bool>(KnownArgs::equip_instantly)) {
        wi->modify_node();
    }
}
