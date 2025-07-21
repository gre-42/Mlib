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

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(json);
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
DECLARE_ARGUMENT(required);
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(team);
DECLARE_ARGUMENT(skills);
DECLARE_ARGUMENT(controller);
DECLARE_ARGUMENT(spawn);
DECLARE_ARGUMENT(player_role);
DECLARE_ARGUMENT(unstuck_mode);
DECLARE_ARGUMENT(behavior);
DECLARE_ARGUMENT(seat);
DECLARE_ARGUMENT(user);
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
DECLARE_ARGUMENT(can_select_opponent);
DECLARE_ARGUMENT(can_select_weapon);
DECLARE_ARGUMENT(velocity_error_std);
DECLARE_ARGUMENT(yaw_error_std);
DECLARE_ARGUMENT(pitch_error_std);
DECLARE_ARGUMENT(error_alpha);
DECLARE_ARGUMENT(respawn_cooldown_time);
}

namespace SpawnKeys {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(group);
DECLARE_ARGUMENT(vehicle);
}

namespace UserKeys {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(name);
}

const std::string LoadPlayers::key = "load_players";

LoadSceneJsonUserFunction LoadPlayers::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    LoadPlayers(args.physics_scene()).execute(args);
};

LoadPlayers::LoadPlayers(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void LoadPlayers::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    // Example JSON file:
    // {
    //     "library": "teams",
    //     "game_mode": "rally",
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
    // macro_playback teams.create_player_and_car_for_pc decimate: player_name:you car_name:_tiger_tank TEAM:red GAME_MODE:racing IF_STYLE: R:1 G:0.8 B:0.8;
    // macro_playback teams.create_player_and_car_for_npc car_name:_tiger_tank decimate: player_name:npc1 TEAM:red  GAME_MODE:racing IF_STYLE: R:1 G:0.8 B:0.8
    //    TEAMS_WAY_POINTS_RESOURCE:TEAMS_WAY_POINTS_RESOURCE;

    try {
        auto filename = args.arguments.path(KnownArgs::json);
        nlohmann::json j;
        {
            auto f = create_ifstream(filename);
            if (f->fail()) {
                THROW_OR_ABORT("Could not open players JSON file \"" + filename + '"');
            }
            *f >> j;
            if (f->fail()) {
                THROW_OR_ABORT("Could not read from \"" + filename + '"');
            }
        }
        JsonView jv{ j };
        jv.validate(ToplevelKeys::options);
        nlohmann::json defaults = jv.at(ToplevelKeys::defaults);
        validate(defaults, PlayerKeys::options);
        nlohmann::json default_skills = defaults.at(PlayerKeys::skills);
        JsonView default_skillsv{ default_skills };
        for (const auto& jplayer : jv.at(ToplevelKeys::players)) {
            JsonView player{jplayer};
            player.validate(PlayerKeys::options);
            if (auto rj = player.try_at(PlayerKeys::required); rj.has_value()) {
                if (!args.macro_line_executor.eval_boolean_expression(*rj)) {
                    continue;
                }
            }
            auto get = [&defaults, &player](std::string_view name){
                if (player.contains(name)) {
                    return player.at(name);
                }
                if (defaults.contains(name)) {
                    return defaults.at(name);
                }
                THROW_OR_ABORT("Could not find key \"" + std::string{ name } + "\" in player or defaults");
            };
            auto get_skill = [&default_skillsv, &player](std::string_view source, std::string_view name){
                auto player_skill = player.try_resolve(PlayerKeys::skills, source, name);
                return player_skill.has_value()
                    ? *player_skill
                    : default_skillsv.resolve(source, name);
            };
            auto team = player.at<std::string>(PlayerKeys::team);
            auto color = jv.resolve<UFixedArray<float, 3>>(ToplevelKeys::teams, team, TeamKeys::style, StyleKeys::color);
            auto spawn_group = player.resolve_default<std::string>("", PlayerKeys::spawn, SpawnKeys::group);
            auto vehicle_name = player.resolve<std::string>(PlayerKeys::spawn, SpawnKeys::vehicle, SpawnedVehicleKeys::type);
            const auto& vars = args.asset_references["vehicles"].at(vehicle_name).rp;
            if (auto controller = player.try_at<std::string>(PlayerKeys::controller); controller.has_value()) {
                nlohmann::json let{
                    {"spawner_name", player.at<std::string>(PlayerKeys::name)},
                    {"player_name", player.at<std::string>(PlayerKeys::name)},
                    {"asset_id", vehicle_name},
                    {"spawn_group", spawn_group},
                    {"team", team},
                    {"player_role", get(PlayerKeys::player_role).get<std::string>()},
                    {"initial_behavior", get(PlayerKeys::behavior).get<std::string>()},
                    {"seat", get(PlayerKeys::seat).get<std::string>()},
                    {"unstuck_mode", get(PlayerKeys::unstuck_mode).get<std::string>()},
                    {"if_human_style", true},
                    {"if_car_body_renderable_style", true},
                    {"color", color},
                    {"user_drive", get_skill(SourceKeys::user, SkillsKeys::can_drive)},
                    {"user_aim", get_skill(SourceKeys::user, SkillsKeys::can_aim)},
                    {"user_shoot", get_skill(SourceKeys::user, SkillsKeys::can_shoot)},
                    {"ai_drive", get_skill(SourceKeys::ai, SkillsKeys::can_drive)},
                    {"ai_aim", get_skill(SourceKeys::ai, SkillsKeys::can_aim)},
                    {"ai_shoot", get_skill(SourceKeys::ai, SkillsKeys::can_shoot)},
                    {"ai_select_opponent", get_skill(SourceKeys::ai, SkillsKeys::can_select_opponent)},
                    {"ai_select_weapon", get_skill(SourceKeys::ai, SkillsKeys::can_select_weapon)},
                    {"velocity_error_std", get_skill(SourceKeys::ai, SkillsKeys::velocity_error_std)},
                    {"yaw_error_std", get_skill(SourceKeys::ai, SkillsKeys::yaw_error_std)},
                    {"pitch_error_std", get_skill(SourceKeys::ai, SkillsKeys::pitch_error_std)},
                    {"error_alpha", get_skill(SourceKeys::ai, SkillsKeys::error_alpha)},
                    {"respawn_cooldown_time", get_skill(SourceKeys::ai, SkillsKeys::respawn_cooldown_time)},
                    {"mute", false}
                };
                if (*controller == "pc") {
                    auto user = player.try_at(PlayerKeys::user);
                    if (!user.has_value()) {
                        THROW_OR_ABORT("\"pc\" controller requires \"user\"");
                    }
                    let["user_id"] = user->at("id");
                    let["user_name"] = user->at("name");
                } else if (*controller != "npc") {
                    THROW_OR_ABORT("Unknown controller: \"" + *controller + "\". Known controllers: \"pc\", \"npc\"");
                }
                
                nlohmann::json line{
                    {
                        MacroKeys::playback,
                        (jv.at(ToplevelKeys::library).get<std::string>() + ".create_player_and_" +
                            vars.database.at<std::string>("vehicle_class") +
                            "_for_" + *controller)
                    },
                    {
                        MacroKeys::let,
                        std::move(let)
                    }
                };
                args.macro_line_executor(line, nullptr);
            } else {
                nlohmann::json line{
                    {
                        MacroKeys::playback,
                        (jv.at(ToplevelKeys::library).get<std::string>() + ".create_spawner_and_" +
                            vars.database.at<std::string>("vehicle_class"))
                    },
                    {
                        MacroKeys::let,
                        {
                            {"spawner_name", player.at<std::string>(PlayerKeys::name)},
                            {"asset_id", vehicle_name},
                            {"spawn_group", spawn_group},
                            {"team", team},
                            {"if_human_style", true},
                            {"if_car_body_renderable_style", true},
                            {"color", color},
                            {"velocity_error_std", get_skill(SourceKeys::ai, SkillsKeys::velocity_error_std)},
                            {"yaw_error_std", get_skill(SourceKeys::ai, SkillsKeys::yaw_error_std)},
                            {"pitch_error_std", get_skill(SourceKeys::ai, SkillsKeys::pitch_error_std)},
                            {"error_alpha", get_skill(SourceKeys::ai, SkillsKeys::error_alpha)},
                            {"respawn_cooldown_time", get_skill(SourceKeys::ai, SkillsKeys::respawn_cooldown_time)},
                            {"mute", false}
                        }
                    }
                };
                args.macro_line_executor(line, nullptr);
            }
        }
    } catch (const nlohmann::detail::exception& e) {
        throw std::runtime_error(e.what());
    }
}
