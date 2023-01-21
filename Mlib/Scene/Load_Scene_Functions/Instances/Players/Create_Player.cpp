#include "Create_Player.hpp"
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Driving_Direction.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

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
        "\\s+game_mode=(\\w+)"
        "\\s+unstuck_mode=(\\w+)"
        "\\s+driving_mode=(\\w+)"
        "\\s+driving_direction=(\\w+)$");
    Mlib::re::smatch match;
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
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto driving_mode = driving_modes.find(match[DRIVING_MODE].str());
    if (driving_mode == driving_modes.end()) {
        THROW_OR_ABORT("Could not find driving mode with name \"" + match[DRIVING_MODE].str() + '"');
    }
    auto player = std::make_unique<Player>(
        scene,
        supply_depots,
        scene_config.physics_engine_config,
        physics_engine.collision_query_,
        players,
        match[NAME].str(),
        match[TEAM].str(),
        game_mode_from_string(match[GAME_MODE].str()),
        unstuck_mode_from_string(match[UNSTUCK_MODE].str()),
        driving_mode->second,
        driving_direction_from_string(match[DRIVING_DIRECTION].str()),
        delete_node_mutex,
        args.ui_focus.focuses);
    Player* p = player.get();
    players.add_player(std::move(player));
    physics_engine.advance_times_.add_advance_time(*p);
    physics_engine.add_external_force_provider(*p);
}
