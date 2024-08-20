#include "Set_Externals_Creator.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Keys.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Physics/Ai/Control_Source.hpp>
#include <Mlib/Physics/Ai/Skill_Map.hpp>
#include <Mlib/Physics/Ai/Skills.hpp>
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
DECLARE_ARGUMENT(EXTERNALS_PLAYER_NAME);
DECLARE_ARGUMENT(IF_PC);
DECLARE_ARGUMENT(IF_MANUAL_AIM);
DECLARE_ARGUMENT(IF_MANUAL_SHOOT);
DECLARE_ARGUMENT(IF_MANUAL_DRIVE);
DECLARE_ARGUMENT(BEHAVIOR);
DECLARE_ARGUMENT(EXTERNALS_ROLE);
}

const std::string SetExternalsCreator::key = "set_externals_creator";

LoadSceneJsonUserFunction SetExternalsCreator::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    args.macro_line_executor.block_arguments().validate_complement(LetKeys::options);
    SetExternalsCreator(args.renderable_scene()).execute(args);
};

SetExternalsCreator::SetExternalsCreator(RenderableScene& renderable_scene)
    : LoadSceneInstanceFunction{ renderable_scene }
{}

void SetExternalsCreator::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto spawner_name = args.arguments.at<std::string>(KnownArgs::spawner);
    auto& spawner = vehicle_spawners.get(spawner_name);
    if (!spawner.has_scene_vehicle()) {
        THROW_OR_ABORT("Spawner \"" + spawner_name + "\" has no vehicle");
    }
    spawner.get_primary_scene_vehicle().set_create_vehicle_externals(
        [macro_line_executor = args.macro_line_executor,
         macro = args.arguments.at(KnownArgs::externals),
         spawner_name](
            const std::string& player_name,
            ExternalsMode externals_mode,
            const std::string& behavior)
        {
            if (externals_mode == ExternalsMode::NONE) {
                THROW_OR_ABORT("Invalid externals mode");
            }
            nlohmann::json let{
                {LetKeys::EXTERNALS_PLAYER_NAME, player_name},
                {LetKeys::IF_PC, (externals_mode == ExternalsMode::PC)},
                {LetKeys::BEHAVIOR, behavior}
            };
            macro_line_executor.inserted_block_arguments(let)(macro, nullptr, nullptr);
        }
    );
    spawner.get_primary_scene_vehicle().set_create_vehicle_internals(
        [macro_line_executor = args.macro_line_executor,
         macro = args.arguments.at(KnownArgs::internals),
         spawner_name](
            const std::string& player_name,
            ExternalsMode externals_mode,
            const SkillMap& skills,
            const std::string& behavior,
            const InternalsMode& internals_mode)
        {
            if (externals_mode == ExternalsMode::NONE) {
                THROW_OR_ABORT("Invalid externals mode");
            }
            nlohmann::json let{
                {LetKeys::EXTERNALS_PLAYER_NAME, player_name},
                {LetKeys::IF_PC, (externals_mode == ExternalsMode::PC)},
                {LetKeys::IF_MANUAL_AIM, skills.skills(ControlSource::USER).can_aim},
                {LetKeys::IF_MANUAL_SHOOT, skills.skills(ControlSource::USER).can_shoot},
                {LetKeys::IF_MANUAL_DRIVE, skills.skills(ControlSource::USER).can_drive},
                {LetKeys::BEHAVIOR, behavior},
                {LetKeys::EXTERNALS_ROLE, internals_mode.role}
            };
            macro_line_executor.inserted_block_arguments(let)(macro, nullptr, nullptr);
        }
    );
}
