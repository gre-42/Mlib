#include "Load_Players.hpp"
#include <Mlib/Json.hpp>
#include <Mlib/Macro_Line_Executor.hpp>
#include <Mlib/Regex_Select.hpp>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(JSON);
DECLARE_OPTION(WAY_POINTS);

LoadSceneInstanceFunction::UserFunction LoadPlayers::user_function = [](const UserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*load_players"
        "\\s+json=([\\w+-. \\(\\)/\\\\:]+)"
        "\\s+way_points=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        LoadPlayers(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

LoadPlayers::LoadPlayers(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void LoadPlayers::execute(
    const std::smatch& match,
    const UserFunctionArgs& args)
{
    // Example JSON file:
    // {
    //     "library": "teams",
    //     "game_mode": "racing",
    //     "teams": {
    //         "red": { "style": { "color": [1, 0.8, 0.8] },
    //         "blue": { "style": { "color": [0.8, 0.8, 1] } }
    //     },
    //     "players": [
    //         {
    //             "controller": "pc", "name": "you", "team": "red",
    //             "spawned_vehicle": { "type": "tiger_tank" }
    //         },
    //         {
    //             "controller": "npc", "name": "npc1", "team": "red",
    //             "spawned_vehicle": { "type": "tiger_tank" }
    //         },
    //         ...
    //     ]
    // }
    //
    // Example macro calls:
    // macro_playback teams.create_player_and_car_for_pc DECIMATE: PLAYER_NAME:you CAR_NAME:_tiger_tank TEAM:red GAME_MODE:racing IF_STYLE: R:1 G:0.8 B:0.8;
    // macro_playback teams.create_player_and_car_for_npc CAR_NAME:_tiger_tank DECIMATE: PLAYER_NAME:npc1 TEAM:red  GAME_MODE:racing IF_STYLE: R:1 G:0.8 B:0.8
    //    TEAMS_WAY_POINTS_RESOURCE:TEAMS_WAY_POINTS_RESOURCE;

    try {
        std::string filename = match[JSON].str();
        json j;
        std::ifstream f{filename};
        if (f.fail()) {
            throw std::runtime_error("Could not open file \"" + filename + '"');
        }
        f >> j;
        if (f.fail()) {
            throw std::runtime_error("Could not read from \"" + filename + '"');
        }
        json defaults = j.at("defaults");
        json default_skills = defaults.at("skills");
        for (const auto& player : j.at("players")) {
            auto get = [&defaults, &player](const std::string& name){
                return player.contains(name)
                    ? player.at(name)
                    : defaults.at(name);
            };
            auto get_skill = [&default_skills, &player](const std::string& name){
                return player.contains("skills") && player.at("skills").contains(name)
                    ? player.at("skills").at(name)
                    : default_skills.at(name);
            };
            std::stringstream sstr;
            std::string team = player.at("team").get<std::string>();
            auto color = get_fixed_array<float, 3>(j.at("teams").at(team).at("style").at("color"));
            auto controller = player.at("controller").get<std::string>();
            sstr << "macro_playback " <<
                j.at("library").get<std::string>() << ".create_player_and_car_for_" << controller <<
                " DECIMATE:"
                " PLAYER_NAME:" << player.at("name").get<std::string>() <<
                " CAR_NAME:_" << player.at("spawned_vehicle").at("type").get<std::string>() <<
                " TEAM:" << team <<
                " GAME_MODE:" << get("game_mode").get<std::string>() <<
                " UNSTUCK_MODE:" << get("unstuck_mode").get<std::string>() <<
                " IF_SET_WAY_POINTS:" << (get("set_way_points").get<bool>() ? "" : "#") <<
                " IF_STYLE:"
                " R:" << color(0) <<
                " G:" << color(1) <<
                " B:" << color(2);
            sstr << " IF_MANUAL_AIM:" << (get_skill("manual_aim").get<bool>() ? "" : "#");
            sstr << " VERROR_STD:" << get_skill("verror_std").get<float>();
            if (controller == "pc") {
                // Do nothing
            } else if (controller == "npc") {
                sstr << " TEAMS_WAY_POINTS_RESOURCE:" << match[WAY_POINTS].str();
            } else {
                throw std::runtime_error("Unknown controller type");
            }
            args.macro_line_executor(sstr.str(), args.local_substitutions, args.rsc);
        }
    } catch (const nlohmann::detail::exception& e) {
        throw std::runtime_error(e.what());
    }
}
