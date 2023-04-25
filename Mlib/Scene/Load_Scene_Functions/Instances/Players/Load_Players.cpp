#include "Load_Players.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/FPath.hpp>
#include <Mlib/Json.hpp>
#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <fstream>

using json = nlohmann::json;

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(json);
DECLARE_ARGUMENT(way_points);
}

const std::string LoadPlayers::key = "load_players";

LoadSceneJsonUserFunction LoadPlayers::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    LoadPlayers(args.renderable_scene()).execute(args);
};

LoadPlayers::LoadPlayers(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void LoadPlayers::execute(const LoadSceneJsonUserFunctionArgs& args)
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
        std::string filename = args.arguments.path(KnownArgs::json);
        json j;
        std::ifstream f{filename};
        if (f.fail()) {
            THROW_OR_ABORT("Could not open players JSON file \"" + filename + '"');
        }
        f >> j;
        if (f.fail()) {
            THROW_OR_ABORT("Could not read from \"" + filename + '"');
        }
        json defaults = j.at("defaults");
        json default_skills = defaults.at("skills");
        for (const auto& player : j.at("players")) {
            auto get = [&defaults, &player](const std::string& name){
                return player.contains(name)
                    ? player.at(name)
                    : defaults.at(name);
            };
            auto get_skill = [&default_skills, &player](const std::string& source, const std::string& name){
                return player.contains("skills") && player.at("skills").contains(source) && player.at("skills").at(source).contains(name)
                    ? player.at("skills").at(source).at(name)
                    : default_skills.at(source).at(name);
            };
            std::string team = player.at("team").get<std::string>();
            auto color = j.at("teams").at(team).at("style").at("color").get<FixedArray<float, 3>>();
            auto controller = player.at("controller").get<std::string>();
            std::string vehicle_name = player.at("spawned_vehicle").at("type").get<std::string>();
            const auto& vars = args.asset_references.get_replacement_parameters("vehicles").at(vehicle_name);
            nlohmann::json line{
                {
                    "playback",
                    (j.at("library").get<std::string>() + ".create_player_and_" +
                        vars.variables.at<std::string>("VEHICLE_CLASS") +
                        "_for_" + controller)
                },
                {
                    "substitutions",
                    {
                        {"DECIMATE", ""},
                        {"PLAYER_NAME", player.at("name").get<std::string>()},
                        {"HUMAN_NAME", "_" + vehicle_name},
                        {"CAR_NAME", "_" + vehicle_name},
                        {"TEAM", team},
                        {"GAME_MODE", get("game_mode").get<std::string>()},
                        {"UNSTUCK_MODE", get("unstuck_mode").get<std::string>()}
                    }
                },
                {
                    "literals",
                    {
                        {"IF_SET_WAY_POINTS", get("set_way_points")},
                        {"IF_HUMAN_STYLE", true},
                        {"IF_CAR_BODY_RENDERABLE_STYLE", true},
                        {"color", color},
                        {"USER_DRIVE", get_skill("user", "can_drive")},
                        {"USER_AIM", get_skill("user", "can_aim")},
                        {"USER_SHOOT",  get_skill("user", "can_shoot")},
                        {"AI_DRIVE",  get_skill("ai", "can_drive")},
                        {"AI_AIM",  get_skill("ai", "can_aim")},
                        {"AI_SHOOT",  get_skill("ai", "can_shoot")},
                        {"AI_SELECT_BEST_WEAPON",  get_skill("ai", "can_select_best_weapon")},
                        {"VELOCITY_ERROR_STD",  get_skill("ai", "velocity_error_std")},
                        {"YAW_ERROR_STD",  get_skill("ai", "yaw_error_std")},
                        {"PITCH_ERROR_STD",  get_skill("ai", "pitch_error_std")},
                        {"ERROR_ALPHA",  get_skill("ai", "error_alpha")},
                        {"TEAMS_WAY_POINTS_RESOURCE", args.arguments.at(KnownArgs::way_points)}
                    }
                }
            };
            args.macro_line_executor(line, nullptr);
        }
    } catch (const nlohmann::detail::exception& e) {
        throw std::runtime_error(e.what());
    }
}
