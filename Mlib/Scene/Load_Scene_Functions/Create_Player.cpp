#include "Create_Player.hpp"
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Driving_Direction.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NAME);
DECLARE_OPTION(TEAM);
DECLARE_OPTION(GAME_MODE);
DECLARE_OPTION(UNSTUCK_MODE);
DECLARE_OPTION(DRIVING_MODE);
DECLARE_OPTION(DRIVING_DIRECTION);

LoadSceneUserFunction CreatePlayer::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*player_create"
        "\\s+name=([\\w+-.]+)"
        "\\s+team=([\\w+-.]+)"
        "\\s+game_mode=(ramming|racing|bystander|pod_bot_npc|pod_bot_pc)"
        "\\s+unstuck_mode=(off|reverse|delete)"
        "\\s+driving_mode=(pedestrian|car_city|car_arena)"
        "\\s+driving_direction=(center|left|right)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreatePlayer(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreatePlayer::CreatePlayer(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreatePlayer::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto player = std::make_unique<Player>(
        scene,
        physics_engine.collision_query_,
        players,
        match[NAME].str(),
        match[TEAM].str(),
        game_mode_from_string(match[GAME_MODE].str()),
        unstuck_mode_from_string(match[UNSTUCK_MODE].str()),
        driving_modes.at(match[DRIVING_MODE].str()),
        driving_direction_from_string(match[DRIVING_DIRECTION].str()),
        delete_node_mutex);
    Player* p = player.get();
    players.add_player(std::move(player));
    physics_engine.advance_times_.add_advance_time(p);
    physics_engine.add_external_force_provider(p);
}
