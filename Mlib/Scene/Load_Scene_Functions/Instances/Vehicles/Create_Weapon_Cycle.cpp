#include "Create_Weapon_Cycle.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Misc/Weapon_Cycle.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(cycle_node);
}

const std::string CreateWeaponCycle::key = "create_weapon_cycle";

LoadSceneJsonUserFunction CreateWeaponCycle::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateWeaponCycle(args.physics_scene()).execute(args);
};

CreateWeaponCycle::CreateWeaponCycle(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void CreateWeaponCycle::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingBaseClassRef<SceneNode> cycle_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::cycle_node), DP_LOC);
    cycle_node->set_node_modifier(std::make_unique<WeaponCycle>());
}
