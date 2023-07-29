#include "Load_Players.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Json/Misc.hpp>
#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Keys.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
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

namespace ToplevelKeys {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(defaults);
DECLARE_ARGUMENT(players);
DECLARE_ARGUMENT(teams);
DECLARE_ARGUMENT(library);
}


namespace PlayerKeys {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(team);
DECLARE_ARGUMENT(skills);
DECLARE_ARGUMENT(controller);
DECLARE_ARGUMENT(spawned_vehicle);
DECLARE_ARGUMENT(game_mode);
DECLARE_ARGUMENT(unstuck_mode);
DECLARE_ARGUMENT(set_way_points);
}

namespace TeamKeys {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(style);
}

namespace StyleKeys {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(color);
}

namespace SpawnedVehicleKeys {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(type);
}

namespace SourceKeys {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(user);
DECLARE_ARGUMENT(ai);
}

namespace SkillsKeys {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(can_drive);
DECLARE_ARGUMENT(can_aim);
DECLARE_ARGUMENT(can_shoot);
DECLARE_ARGUMENT(can_select_best_weapon);
DECLARE_ARGUMENT(velocity_error_std);
DECLARE_ARGUMENT(yaw_error_std);
DECLARE_ARGUMENT(pitch_error_std);
DECLARE_ARGUMENT(error_alpha);
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
        auto filename = args.arguments.path(KnownArgs::json);
        json j;
        std::ifstream f{filename};
        if (f.fail()) {
            THROW_OR_ABORT("Could not open players JSON file \"" + filename + '"');
        }
        f >> j;
        if (f.fail()) {
            THROW_OR_ABORT("Could not read from \"" + filename + '"');
        }
        JsonView jv{j};
        jv.validate(ToplevelKeys::options);
        json defaults = jv.at(ToplevelKeys::defaults);
        validate(defaults, PlayerKeys::options);
        json default_skills = defaults.at(PlayerKeys::skills);
        for (const auto& jplayer : jv.at(ToplevelKeys::players)) {
            JsonView player{jplayer};
            player.validate(PlayerKeys::options);
            auto get = [&defaults, &player](const std::string& name){
                return player.contains(name)
                    ? player.at(name)
                    : defaults.at(name);
            };
            auto get_skill = [&default_skills, &player](const std::string& source, const std::string& name){
                auto skills = player.try_at(PlayerKeys::skills);
                return player.contains(PlayerKeys::skills) && player.at(PlayerKeys::skills).contains(source) && player.at(PlayerKeys::skills).at(source).contains(name)
                    ? player.at(PlayerKeys::skills).at(source).at(name)
                    : default_skills.at(source).at(name);
            };
            auto team = player.at<std::string>(PlayerKeys::team);
            auto color = jv.at(ToplevelKeys::teams).at(team).at(TeamKeys::style).at(StyleKeys::color).get<FixedArray<float, 3>>();
            auto vehicle_name = player.at(PlayerKeys::spawned_vehicle).at(SpawnedVehicleKeys::type).get<std::string>();
            const auto& vars = args.asset_references.get_replacement_parameters("vehicles").at(vehicle_name).rp;
            if (player.contains(PlayerKeys::controller)) {
                auto controller = player.at<std::string>(PlayerKeys::controller);
                nlohmann::json line{
                    {
                        MacroKeys::playback,
                        (jv.at(ToplevelKeys::library).get<std::string>() + ".create_player_and_" +
                            vars.database.at<std::string>("VEHICLE_CLASS") +
                            "_for_" + controller)
                    },
                    {
                        MacroKeys::literals,
                        {
                            {"DECIMATE", ""},
                            {"SPAWNER_NAME", player.at<std::string>(PlayerKeys::name)},
                            {"PLAYER_NAME", player.at<std::string>(PlayerKeys::name)},
                            {"HUMAN_NAME", vehicle_name},
                            {"CAR_NAME", vehicle_name},
                            {"TEAM", team},
                            {"GAME_MODE", get(PlayerKeys::game_mode).get<std::string>()},
                            {"UNSTUCK_MODE", get(PlayerKeys::unstuck_mode).get<std::string>()},
                            {"IF_SET_WAY_POINTS", get(PlayerKeys::set_way_points)},
                            {"IF_HUMAN_STYLE", true},
                            {"IF_CAR_BODY_RENDERABLE_STYLE", true},
                            {"COLOR", color},
                            {"USER_DRIVE", get_skill(SourceKeys::user, SkillsKeys::can_drive)},
                            {"USER_AIM", get_skill(SourceKeys::user, SkillsKeys::can_aim)},
                            {"USER_SHOOT", get_skill(SourceKeys::user, SkillsKeys::can_shoot)},
                            {"AI_DRIVE", get_skill(SourceKeys::ai, SkillsKeys::can_drive)},
                            {"AI_AIM", get_skill(SourceKeys::ai, SkillsKeys::can_aim)},
                            {"AI_SHOOT", get_skill(SourceKeys::ai, SkillsKeys::can_shoot)},
                            {"AI_SELECT_BEST_WEAPON", get_skill(SourceKeys::ai, SkillsKeys::can_select_best_weapon)},
                            {"VELOCITY_ERROR_STD", get_skill(SourceKeys::ai, SkillsKeys::velocity_error_std)},
                            {"YAW_ERROR_STD", get_skill(SourceKeys::ai, SkillsKeys::yaw_error_std)},
                            {"PITCH_ERROR_STD", get_skill(SourceKeys::ai, SkillsKeys::pitch_error_std)},
                            {"ERROR_ALPHA", get_skill(SourceKeys::ai, SkillsKeys::error_alpha)},
                            {"TEAMS_WAY_POINTS_RESOURCE", args.arguments.at(KnownArgs::way_points)}
                        }
                    }
                };
                args.macro_line_executor(JsonView{line}, nullptr, nullptr);
            } else {
                nlohmann::json line{
                    {
                        MacroKeys::playback,
                        (jv.at(ToplevelKeys::library).get<std::string>() + ".create_spawner_and_" +
                            vars.database.at<std::string>("VEHICLE_CLASS"))
                    },
                    {
                        MacroKeys::literals,
                        {
                            {"DECIMATE", ""},
                            {"SPAWNER_NAME", player.at<std::string>(PlayerKeys::name)},
                            {"HUMAN_NAME", vehicle_name},
                            {"CAR_NAME", vehicle_name},
                            {"TEAM", team},
                            {"IF_HUMAN_STYLE", true},
                            {"IF_CAR_BODY_RENDERABLE_STYLE", true},
                            {"COLOR", color},
                            {"VELOCITY_ERROR_STD", get_skill(SourceKeys::ai, SkillsKeys::velocity_error_std)},
                            {"YAW_ERROR_STD", get_skill(SourceKeys::ai, SkillsKeys::yaw_error_std)},
                            {"PITCH_ERROR_STD", get_skill(SourceKeys::ai, SkillsKeys::pitch_error_std)},
                            {"ERROR_ALPHA", get_skill(SourceKeys::ai, SkillsKeys::error_alpha)}
                        }
                    }
                };
                args.macro_line_executor(JsonView{line}, nullptr, nullptr);
            }
        }
    } catch (const nlohmann::detail::exception& e) {
        throw std::runtime_error(e.what());
    }
}
