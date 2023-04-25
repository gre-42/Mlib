#include "Reset_Supply_Depot_Cooldowns.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Players/Game_Logic/Supply_Depots.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>

using namespace Mlib;

const std::string ResetSupplyDepotCooldowns::key = "reset_supply_depot_cooldowns";

LoadSceneJsonUserFunction ResetSupplyDepotCooldowns::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate({});
    ResetSupplyDepotCooldowns(args.renderable_scene()).execute(args);
};

ResetSupplyDepotCooldowns::ResetSupplyDepotCooldowns(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void ResetSupplyDepotCooldowns::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    supply_depots.reset_cooldown();
}
