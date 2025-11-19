#include "Set_Externals_Creator.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Keys.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Physics/Ai/Control_Source.hpp>
#include <Mlib/Physics/Ai/Skill_Map.hpp>
#include <Mlib/Physics/Ai/Skills.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Remote_Sites.hpp>
#include <Mlib/Players/Containers/Vehicle_Spawners.hpp>
#include <Mlib/Players/Scene_Vehicle/Externals_Mode.hpp>
#include <Mlib/Players/Scene_Vehicle/Internals_Mode.hpp>
#include <Mlib/Players/Scene_Vehicle/Scene_Vehicle.hpp>
#include <Mlib/Players/Scene_Vehicle/Vehicle_Spawner.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgsUnsafe {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(spawner);
DECLARE_ARGUMENT(externals);
DECLARE_ARGUMENT(internals);
}

namespace KnownArgsSafe {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(spawner);
DECLARE_ARGUMENT(asset_id);
}

namespace LetKeys {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(full_user_name);
DECLARE_ARGUMENT(local_user_id);
DECLARE_ARGUMENT(player_name);
DECLARE_ARGUMENT(if_pc);
DECLARE_ARGUMENT(if_manual_aim);
DECLARE_ARGUMENT(if_manual_shoot);
DECLARE_ARGUMENT(if_manual_drive);
DECLARE_ARGUMENT(if_weapon_cycle);
DECLARE_ARGUMENT(behavior);
DECLARE_ARGUMENT(externals_seat);
}

SetExternalsCreator::SetExternalsCreator(
    PhysicsScene& physics_scene,
    const MacroLineExecutor& macro_line_executor)
    : LoadPhysicsSceneInstanceFunction{ physics_scene, &macro_line_executor }
{}

static bool get_if_pc(ExternalsMode externals_mode, const UserInfo* user_info) {
    return
        (externals_mode == ExternalsMode::PC) &&
        (user_info != nullptr) &&
        (user_info->type == UserType::LOCAL);
}

void SetExternalsCreator::execute_unsafe(
    SceneVehicle& vehicle,
    nlohmann::json externals,
    nlohmann::json internals)
{
    macro_line_executor.block_arguments().validate_complement(LetKeys::options);

    vehicle.set_create_vehicle_externals(
        [mle=macro_line_executor, macro=std::move(externals)](
            const Player& player,
            ExternalsMode externals_mode)
        {
            if (externals_mode == ExternalsMode::NONE) {
                THROW_OR_ABORT("Invalid externals mode");
            }
            auto user_info = player.user_info();
            nlohmann::json let{
                {LetKeys::player_name, player.id()},
                {LetKeys::if_pc, get_if_pc(externals_mode, user_info.get())},
                {LetKeys::behavior, player.behavior()}
            };
            if (user_info != nullptr) {
                let[LetKeys::full_user_name] = user_info->full_name;
                if (user_info->type == UserType::LOCAL) {
                    let[LetKeys::local_user_id] = user_info->user_id;
                }
            }
            mle.inserted_block_arguments(std::move(let))(macro, nullptr);
        }
    );
    vehicle.set_create_vehicle_internals(
        [mle=macro_line_executor, macro=std::move(internals)](
            const Player& player,
            const InternalsMode& internals_mode)
        {
            if (player.externals_mode() == ExternalsMode::NONE) {
                THROW_OR_ABORT("Invalid externals mode");
            }
            auto user_info = player.user_info();
            nlohmann::json let{
                {LetKeys::player_name, player.id()},
                {LetKeys::if_pc, get_if_pc(player.externals_mode(), user_info.get())},
                {LetKeys::if_manual_aim, player.skills(ControlSource::USER).can_aim},
                {LetKeys::if_manual_shoot, player.skills(ControlSource::USER).can_shoot},
                {LetKeys::if_manual_drive, player.skills(ControlSource::USER).can_drive},
                {LetKeys::if_weapon_cycle, player.has_weapon_cycle()},
                {LetKeys::behavior, player.behavior()},
                {LetKeys::externals_seat, internals_mode.seat}
            };
            if (user_info != nullptr) {
                let[LetKeys::full_user_name] = user_info->full_name;
                if (user_info->type == UserType::LOCAL) {
                    let[LetKeys::local_user_id] = user_info->user_id;
                }
            }
            mle.inserted_block_arguments(std::move(let))(macro, nullptr);
        }
    );
}

void SetExternalsCreator::execute_safe(
    SceneVehicle& vehicle,
    const std::string& asset_id)
{
    auto externals = std::map<std::string, std::string>{
        {"playback", "create_player_vehicle_externals_" + asset_id}};
    auto internals = std::map<std::string, std::string>{
        {"playback", "create_player_vehicle_internals_" + asset_id}};
    execute_unsafe(
        vehicle,
        externals,
        internals);
}

void SetExternalsCreator::execute_unsafe(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgsUnsafe::options);
    auto spawner_name = args.arguments.at<VariableAndHash<std::string>>(KnownArgsUnsafe::spawner);
    auto& spawner = vehicle_spawners.get(spawner_name);
    if (!spawner.has_scene_vehicle()) {
        THROW_OR_ABORT("Spawner \"" + *spawner_name + "\" has no vehicle");
    }
    execute_unsafe(
        spawner.get_primary_scene_vehicle().get(),
        args.arguments.at(KnownArgsUnsafe::externals),
        args.arguments.at(KnownArgsUnsafe::internals));
}

void SetExternalsCreator::execute_safe(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgsSafe::options);
    auto spawner_name = args.arguments.at<VariableAndHash<std::string>>(KnownArgsUnsafe::spawner);
    auto& spawner = vehicle_spawners.get(spawner_name);
    if (!spawner.has_scene_vehicle()) {
        THROW_OR_ABORT("Spawner \"" + *spawner_name + "\" has no vehicle");
    }
    execute_safe(
        spawner.get_primary_scene_vehicle().get(),
        args.arguments.at<std::string>(KnownArgsSafe::asset_id));
}

namespace {

struct RegisterJsonUserFunction0 {
    RegisterJsonUserFunction0() {
        LoadSceneFuncs::register_json_user_function(
            "set_externals_creator_unsafe",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                SetExternalsCreator(args.physics_scene(), args.macro_line_executor).execute_unsafe(args);
            });
    }
} obj0;

struct RegisterJsonUserFunction1 {
    RegisterJsonUserFunction1() {
        LoadSceneFuncs::register_json_user_function(
            "set_externals_creator_safe",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                SetExternalsCreator(args.physics_scene(), args.macro_line_executor).execute_safe(args);
            });
    }
} obj1;

}
