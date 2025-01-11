#include "Create_Scene.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Containers/Race_Identifier.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Logics/Clear_Mode.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Scene/Renderable_Scenes.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Time/Fps/Realtime_Dependent_Fps.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(world);
DECLARE_ARGUMENT(focus_mask);
DECLARE_ARGUMENT(submenus);
DECLARE_ARGUMENT(fly);
DECLARE_ARGUMENT(rotate);
DECLARE_ARGUMENT(depth_fog);
DECLARE_ARGUMENT(low_pass);
DECLARE_ARGUMENT(high_pass);
DECLARE_ARGUMENT(bloom_iterations);
DECLARE_ARGUMENT(bloom_thresholds);
DECLARE_ARGUMENT(with_skybox);
DECLARE_ARGUMENT(with_flying_logic);
DECLARE_ARGUMENT(clear_mode);
DECLARE_ARGUMENT(max_tracks);
DECLARE_ARGUMENT(save_playback);
}

const std::string CreateScene::key = "create_scene";

LoadSceneJsonUserFunction CreateScene::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);

    auto name = args.arguments.at<std::string>(KnownArgs::name);
    auto [_, state] = args.renderable_scenes.try_emplace(
        name,
        name,
        args.arguments.at<std::string>(KnownArgs::world),
        name + ".rendering_resources",
        args.scene_config.render_config.anisotropic_filtering_level,
        RenderingContextStack::primary_scene_node_resources(),
        RenderingContextStack::primary_particle_resources(),
        RenderingContextStack::primary_trail_resources(),
        args.surface_contact_db,
        args.dynamic_light_db,
        args.scene_config,
        args.button_states,
        args.cursor_states,
        args.scroll_wheel_states,
        args.ui_focus,
        SceneConfigResource{
            .fly = args.arguments.at<bool>(KnownArgs::fly),
            .rotate = args.arguments.at<bool>(KnownArgs::rotate),
            .depth_fog = args.arguments.at<bool>(KnownArgs::depth_fog),
            .low_pass = args.arguments.at<bool>(KnownArgs::low_pass),
            .high_pass = args.arguments.at<bool>(KnownArgs::high_pass),
            .bloom_iterations = args.arguments.at<UFixedArray<unsigned int, 2>>(KnownArgs::bloom_iterations),
            .bloom_thresholds = args.arguments.at<UFixedArray<float, 3>>(KnownArgs::bloom_thresholds),
            .with_skybox = args.arguments.at<bool>(KnownArgs::with_skybox),
            .with_flying_logic = args.arguments.at<bool>(KnownArgs::with_flying_logic),
            .background_color = {1.f, 0.f, 1.f},
            .clear_mode = clear_mode_from_string(args.arguments.at<std::string>(KnownArgs::clear_mode))},
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
        args.render_set_fps.ds);
    if (state == InsertionStatus::FAILURE_NAME_COLLISION) {
        THROW_OR_ABORT("Scene with name \"" + name + "\" already exists");
    }
    if (state == InsertionStatus::FAILURE_SHUTDOWN) {
        THROW_OR_ABORT("Attempt to create scene with name \"" + name + "\" during shutdown");
    }
    if (state != InsertionStatus::SUCCESS) {
        THROW_OR_ABORT("Unknown state after creating scene with name \"" + name + '"');
    }
};
