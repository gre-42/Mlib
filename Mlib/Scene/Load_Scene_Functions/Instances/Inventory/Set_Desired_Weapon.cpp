#include "Set_Desired_Weapon.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Weapon_Cycle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Misc/Weapon_Cycle.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(cycle_node);
DECLARE_ARGUMENT(weapon);
DECLARE_ARGUMENT(equip_instantly);
}

const std::string SetDesiredWeapon::key = "set_desired_weapon";

LoadSceneJsonUserFunction SetDesiredWeapon::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SetDesiredWeapon(args.physics_scene()).execute(args);
};

SetDesiredWeapon::SetDesiredWeapon(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void SetDesiredWeapon::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingBaseClassRef<SceneNode> cycle_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::cycle_node), DP_LOC);
    auto player_name = args.arguments.try_at_non_null<VariableAndHash<std::string>>(KnownArgs::player);
    std::string weapon_name = args.arguments.at<std::string>(KnownArgs::weapon);
    auto& wc = get_weapon_cycle(cycle_node);
    wc.set_desired_weapon(player_name, weapon_name);
    if (args.arguments.at<bool>(KnownArgs::equip_instantly)) {
        wc.modify_node();
    }
}
