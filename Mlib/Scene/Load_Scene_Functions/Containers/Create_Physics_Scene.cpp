#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Translator.hpp>
#include <Mlib/Physics/Containers/Race_Identifier.hpp>
#include <Mlib/Remote/Remote_Params.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Physics_Scene.hpp>
#include <Mlib/Scene/Physics_Scenes.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Time/Fps/Realtime_Dependent_Fps.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(world);
DECLARE_ARGUMENT(max_tracks);
DECLARE_ARGUMENT(save_playback);
DECLARE_ARGUMENT(gid);
DECLARE_ARGUMENT(primary_user_id);
DECLARE_ARGUMENT(remote_params);
}

using namespace Mlib;

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "create_physics_scene",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);

                auto name = args.arguments.at<std::string>(KnownArgs::name);
                auto [_, state] = args.physics_scenes.try_emplace(
                    name,
                    name,
                    args.arguments.at<VariableAndHash<std::string>>(KnownArgs::world),
                    name + ".rendering_resources",
                    args.scene_config.render_config.anisotropic_filtering_level,
                    args.scene_config,
                    args.external_json_macro_arguments,
                    args.remote_sites,
                    args.asset_references,
                    RenderingContextStack::primary_scene_node_resources(),
                    RenderingContextStack::primary_particle_resources(),
                    RenderingContextStack::primary_trail_resources(),
                    args.surface_contact_db,
                    args.bullet_property_db,
                    args.dynamic_light_db,
                    args.arguments.at<size_t>(KnownArgs::max_tracks, 0),
                    args.arguments.at<bool>(KnownArgs::save_playback, false),
                    RaceIdentifier{
                        .level = "",
                        .session = "",
                        .laps = 0,
                        .milliseconds = 0},
                    args.render_set_fps.ds,
                    args.ui_focuses[args.arguments.at<uint32_t>(KnownArgs::primary_user_id)],
                    std::make_unique<Translator>(args.translators, VariableAndHash{ args.arguments.at<AssetGroupAndId>(KnownArgs::gid) }),
                    args.arguments.try_at_non_null<RemoteParams>(KnownArgs::remote_params));
                if (state == InsertionStatus::FAILURE_NAME_COLLISION) {
                    THROW_OR_ABORT("Scene with name \"" + name + "\" already exists");
                }
                if (state == InsertionStatus::FAILURE_SHUTDOWN) {
                    THROW_OR_ABORT("Attempt to create scene with name \"" + name + "\" during shutdown");
                }
                if (state != InsertionStatus::SUCCESS) {
                    THROW_OR_ABORT("Unknown state after creating scene with name \"" + name + '"');
                }
            });
    }
} obj;

}
