#include "Team_Set_Waypoint.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Load_Scene_User_Function_Args.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(TEAM);
DECLARE_OPTION(POSITION_X);
DECLARE_OPTION(POSITION_Y);
DECLARE_OPTION(POSITION_Z);

const std::string TeamSetWaypoint::key = "team_set_waypoint";

LoadSceneUserFunction TeamSetWaypoint::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^team-name=([\\w+-.]+)"
        "\\s+position=([\\w+-.]*) ([\\w+-.]*) ([\\w+-.]*)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    TeamSetWaypoint(args.renderable_scene()).execute(match, args);
};

TeamSetWaypoint::TeamSetWaypoint(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void TeamSetWaypoint::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    players.set_team_waypoint(
        match[TEAM].str(), {
            safe_stof(match[POSITION_X].str()),
            safe_stof(match[POSITION_Y].str()),
            safe_stof(match[POSITION_Z].str())});
}
