#include "Reset_Supply_Depot_Cooldowns.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Players/Game_Logic/Supply_Depots.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
}

ResetSupplyDepotCooldowns::ResetSupplyDepotCooldowns(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void ResetSupplyDepotCooldowns::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    supply_depots.reset_cooldown();
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "reset_supply_depot_cooldowns",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                ResetSupplyDepotCooldowns{args.physics_scene()}.execute(args);
            });
    }
} obj;

}
