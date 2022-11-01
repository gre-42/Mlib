#include "Set_Objective.hpp"
#include <Mlib/Players/Advance_Times/Game_Logic.hpp>
#include <Mlib/Players/Game_Logic/Objective.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(OBJECTIVE);

LoadSceneUserFunction SetObjective::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_objective"
        "\\s+([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        SetObjective(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

SetObjective::SetObjective(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetObjective::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    game_logic.team_deathmatch.set_objective(
        objective_from_string(match[OBJECTIVE].str()));
}
