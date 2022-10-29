#include "Reset_Supply_Depot_Cooldowns.hpp"
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Players/Game_Logic/Supply_Depots.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(PLAYER);
DECLARE_OPTION(MACRO);

LoadSceneUserFunction ResetSupplyDepotCooldowns::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*reset_supply_depot_cooldowns$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        ResetSupplyDepotCooldowns(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

ResetSupplyDepotCooldowns::ResetSupplyDepotCooldowns(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void ResetSupplyDepotCooldowns::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    supply_depots.reset_cooldown();
}
