#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Focus_Filter.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Translator.hpp>
#include <Mlib/Physics/Containers/Race_Identifier.hpp>
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
DECLARE_ARGUMENT(focus_mask);
DECLARE_ARGUMENT(submenus);
DECLARE_ARGUMENT(max_tracks);
DECLARE_ARGUMENT(save_playback);
DECLARE_ARGUMENT(gid);
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
                    RenderingContextStack::primary_scene_node_resources(),
                    RenderingContextStack::primary_particle_resources(),
                    RenderingContextStack::primary_trail_resources(),
                    args.surface_contact_db,
                    args.dynamic_light_db,
                    args.arguments.at<size_t>(KnownArgs::max_tracks, 0),
                    args.arguments.at<bool>(KnownArgs::save_playback, false),
                    RaceIdentifier{
                        .level = "",
                        .session = "",
                        .laps = 0,
                        .milliseconds = 0},
                    FocusFilter{
                        .focus_mask = focus_from_string(args.arguments.at<std::string>(KnownArgs::focus_mask)),
                        .submenu_ids = args.arguments.at_non_null<std::set<std::string>>(KnownArgs::submenus, {})},
                    args.render_set_fps.ds,
                    std::make_unique<Translator>(args.translators, VariableAndHash{ args.arguments.at<AssetGroupAndId>(KnownArgs::gid) }));
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
