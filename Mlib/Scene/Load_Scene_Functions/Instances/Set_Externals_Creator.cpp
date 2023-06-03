#include "Set_Externals_Creator.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Keys.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Players/Containers/Vehicle_Spawners.hpp>
#include <Mlib/Players/Scene_Vehicle/Control_Source.hpp>
#include <Mlib/Players/Scene_Vehicle/Externals_Mode.hpp>
#include <Mlib/Players/Scene_Vehicle/Scene_Vehicle.hpp>
#include <Mlib/Players/Scene_Vehicle/Vehicle_Spawner.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(spawner);
DECLARE_ARGUMENT(macro);
DECLARE_ARGUMENT(capture);
}

const std::string SetExternalsCreator::key = "set_externals_creator";

LoadSceneJsonUserFunction SetExternalsCreator::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SetExternalsCreator(args.renderable_scene()).execute(args);
};

SetExternalsCreator::SetExternalsCreator(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetExternalsCreator::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto capture = args.arguments.contains(KnownArgs::capture)
        ? args.arguments.child(KnownArgs::capture)
        : JsonMacroArguments();
    std::string spawner_name = args.arguments.at<std::string>(KnownArgs::spawner);
    auto& spawner = vehicle_spawners.get(spawner_name);
    if (!spawner.has_scene_vehicle()) {
        THROW_OR_ABORT("Spawner \"" + spawner_name + "\" has no vehicle");
    }
    spawner.get_primary_scene_vehicle().set_create_externals(
        [macro_line_executor = args.macro_line_executor,
         macro = args.arguments.at(KnownArgs::macro),
         spawner_name,
         capture](
            const std::string& player_name,
            ExternalsMode externals_mode,
            const std::unordered_map<ControlSource, Skills>& skills)
        {
            if (externals_mode == ExternalsMode::NONE) {
                THROW_OR_ABORT("Invalid externals mode");
            }
            JsonMacroArguments local_args{capture};
            local_args.insert_json(nlohmann::json{
                {"SPAWNER_NAME", spawner_name},
                {"PLAYER_NAME", player_name},
                {"IF_PC", (externals_mode == ExternalsMode::PC)},
                {"IF_MANUAL_AIM", skills.at(ControlSource::USER).can_aim},
                {"IF_MANUAL_SHOOT", skills.at(ControlSource::USER).can_shoot},
                {"IF_MANUAL_DRIVE", skills.at(ControlSource::USER).can_drive}
            });
            macro_line_executor(
                JsonView{macro},
                &local_args,
                nullptr);
        }
    );
}
