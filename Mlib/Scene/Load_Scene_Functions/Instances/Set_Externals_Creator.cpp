#include "Set_Externals_Creator.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Keys.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Physics/Ai/Control_Source.hpp>
#include <Mlib/Physics/Ai/Skill_Map.hpp>
#include <Mlib/Physics/Ai/Skills.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Vehicle_Spawners.hpp>
#include <Mlib/Players/Scene_Vehicle/Externals_Mode.hpp>
#include <Mlib/Players/Scene_Vehicle/Internals_Mode.hpp>
#include <Mlib/Players/Scene_Vehicle/Scene_Vehicle.hpp>
#include <Mlib/Players/Scene_Vehicle/Vehicle_Spawner.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(spawner);
DECLARE_ARGUMENT(externals);
DECLARE_ARGUMENT(internals);
}

namespace LetKeys {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(user_id);
DECLARE_ARGUMENT(user_name);
DECLARE_ARGUMENT(player_name);
DECLARE_ARGUMENT(if_pc);
DECLARE_ARGUMENT(if_manual_aim);
DECLARE_ARGUMENT(if_manual_shoot);
DECLARE_ARGUMENT(if_manual_drive);
DECLARE_ARGUMENT(if_weapon_cycle);
DECLARE_ARGUMENT(behavior);
DECLARE_ARGUMENT(externals_seat);
}

const std::string SetExternalsCreator::key = "set_externals_creator";

LoadSceneJsonUserFunction SetExternalsCreator::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    args.macro_line_executor.block_arguments().validate_complement(LetKeys::options);
    SetExternalsCreator(args.physics_scene()).execute(args);
};

SetExternalsCreator::SetExternalsCreator(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void SetExternalsCreator::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto spawner_name = args.arguments.at<std::string>(KnownArgs::spawner);
    auto& spawner = vehicle_spawners.get(spawner_name);
    if (!spawner.has_scene_vehicle()) {
        THROW_OR_ABORT("Spawner \"" + spawner_name + "\" has no vehicle");
    }
    spawner.get_primary_scene_vehicle()->set_create_vehicle_externals(
        [macro_line_executor = args.macro_line_executor,
         macro = args.arguments.at(KnownArgs::externals),
         spawner_name](
            const Player& player,
            ExternalsMode externals_mode)
        {
            if (externals_mode == ExternalsMode::NONE) {
                THROW_OR_ABORT("Invalid externals mode");
            }
            nlohmann::json let{
                {LetKeys::user_id, player.user_id()},
                {LetKeys::user_name, player.user_name()},
                {LetKeys::player_name, player.id()},
                {LetKeys::if_pc, (externals_mode == ExternalsMode::PC)},
                {LetKeys::behavior, player.behavior()}
            };
            macro_line_executor.inserted_block_arguments(let)(macro, nullptr);
        }
    );
    spawner.get_primary_scene_vehicle()->set_create_vehicle_internals(
        [macro_line_executor = args.macro_line_executor,
         macro = args.arguments.at(KnownArgs::internals),
         spawner_name](
            const Player& player,
            const InternalsMode& internals_mode)
        {
            if (player.externals_mode() == ExternalsMode::NONE) {
                THROW_OR_ABORT("Invalid externals mode");
            }
            nlohmann::json let{
                {LetKeys::player_name, player.id()},
                {LetKeys::if_pc, (player.externals_mode() == ExternalsMode::PC)},
                {LetKeys::if_manual_aim, player.skills(ControlSource::USER).can_aim},
                {LetKeys::if_manual_shoot, player.skills(ControlSource::USER).can_shoot},
                {LetKeys::if_manual_drive, player.skills(ControlSource::USER).can_drive},
                {LetKeys::if_weapon_cycle, player.has_weapon_cycle()},
                {LetKeys::behavior, player.behavior()},
                {LetKeys::externals_seat, internals_mode.seat}
            };
            if (player.user_id() != UINT32_MAX) {
                let[LetKeys::user_id] = player.user_id();
                let[LetKeys::user_name] = player.user_name();
            }
            macro_line_executor.inserted_block_arguments(let)(macro, nullptr);
        }
    );
}
