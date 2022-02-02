#include "Team_Set_Waypoint.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(TEAM);
DECLARE_OPTION(POSITION_X);
DECLARE_OPTION(POSITION_Y);
DECLARE_OPTION(POSITION_Z);

LoadSceneInstanceFunction::UserFunction TeamSetWaypoint::user_function = [](const UserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*team_set_waypoint"
        "\\s+team-name=([\\w+-.]+)"
        "\\s+position=([\\w+-.]*) ([\\w+-.]*) ([\\w+-.]*)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        TeamSetWaypoint(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

TeamSetWaypoint::TeamSetWaypoint(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void TeamSetWaypoint::execute(
    const std::smatch& match,
    const UserFunctionArgs& args)
{
    players.set_team_waypoint(
        match[TEAM].str(), {
            safe_stof(match[POSITION_X].str()),
            safe_stof(match[POSITION_Y].str()),
            safe_stof(match[POSITION_Z].str())});
}
