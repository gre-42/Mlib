#include "Create_Weapon_Cycle.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Physics/Misc/Weapon_Cycle.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <stdexcept>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(cycle_node);
}

CreateWeaponCycle::CreateWeaponCycle(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void CreateWeaponCycle::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    DanglingBaseClassRef<SceneNode> cycle_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::cycle_node), CURRENT_SOURCE_LOCATION);
    cycle_node->set_node_modifier(std::make_unique<WeaponCycle>());
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "create_weapon_cycle",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                CreateWeaponCycle{args.physics_scene()}.execute(args);
            });
    }
} obj;

}
