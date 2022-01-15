#include "Create_Player.hpp"
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene_Graph/Driving_Direction.cpp>
#include <Mlib/Scene_Graph/Scene.hpp>

using namespace Mlib;

LoadSceneInstanceFunction::UserFunction CreatePlayer::user_function = [](
    const std::string& line,
    const std::function<RenderableScene&()>& renderable_scene,
    const std::function<FPath(const std::string&)>& fpath,
    const MacroLineExecutor& macro_line_executor,
    SubstitutionMap& external_substitutions,
    SubstitutionMap* local_substitutions,
    RegexSubstitutionCache& rsc)
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
    if (Mlib::re::regex_match(line, match, regex)) {
        CreatePlayer(renderable_scene()).execute(
            match,
            fpath,
            macro_line_executor,
            local_substitutions,
            rsc);
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
    const std::function<FPath(const std::string&)>& fpath,
    const MacroLineExecutor& macro_line_executor,
    SubstitutionMap* local_substitutions,
    RegexSubstitutionCache& rsc)
{
    auto player = std::make_unique<Player>(
        scene,
        physics_engine.collision_query_,
        players,
        match[1].str(),
        match[2].str(),
        game_mode_from_string(match[3].str()),
        unstuck_mode_from_string(match[4].str()),
        driving_modes.at(match[5].str()),
        driving_direction_from_string(match[6].str()),
        delete_node_mutex);
    Player* p = player.get();
    players.add_player(std::move(player));
    physics_engine.advance_times_.add_advance_time(p);
    physics_engine.add_external_force_provider(p);
}
