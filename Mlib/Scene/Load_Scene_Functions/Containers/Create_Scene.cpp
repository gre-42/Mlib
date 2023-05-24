#include "Create_Scene.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Physics/Containers/Race_Identifier.hpp>
#include <Mlib/Render/Batch_Renderers/Aggregate_Array_Renderer.hpp>
#include <Mlib/Render/Batch_Renderers/Array_Instances_Renderer.hpp>
#include <Mlib/Render/Batch_Renderers/Array_Instances_Renderers.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Logics/Clear_Mode.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Scene/Renderable_Scenes.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene_Graph/Batch_Renderers/IAggregate_Renderer.hpp>
#include <Mlib/Scene_Graph/Batch_Renderers/IInstances_Renderer.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(z_order);
DECLARE_ARGUMENT(focus_mask);
DECLARE_ARGUMENT(submenus);
DECLARE_ARGUMENT(fly);
DECLARE_ARGUMENT(rotate);
DECLARE_ARGUMENT(print_gamepad_buttons);
DECLARE_ARGUMENT(depth_fog);
DECLARE_ARGUMENT(low_pass);
DECLARE_ARGUMENT(high_pass);
DECLARE_ARGUMENT(with_skybox);
DECLARE_ARGUMENT(with_flying_logic);
DECLARE_ARGUMENT(clear_mode);
DECLARE_ARGUMENT(max_tracks);
DECLARE_ARGUMENT(setup_new_round);
}

const std::string CreateScene::key = "create_scene";

LoadSceneJsonUserFunction CreateScene::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);

    std::string name = args.arguments.at<std::string>(KnownArgs::name);
    auto rrg = RenderingContextGuard::layer(
        RenderingContextStack::primary_scene_node_resources(),
        RenderingContextStack::primary_particle_resources(),
        name + ".rendering_resources",
        args.scene_config.render_config.anisotropic_filtering_level,
        args.arguments.at<int>(KnownArgs::z_order));
    AggregateRendererGuard arg{
        std::make_shared<AggregateArrayRenderer>(),
        std::make_shared<AggregateArrayRenderer>()};
    InstancesRendererGuard irg{
        std::make_shared<ArrayInstancesRenderers>(),
        std::make_shared<ArrayInstancesRenderer>()};
    if (!args.renderable_scenes.try_emplace(
        name,
        RenderingContextStack::primary_scene_node_resources(),
        RenderingContextStack::primary_particle_resources(),
        args.surface_contact_db,
        args.scene_config,
        args.button_states,
        args.cursor_states,
        args.scroll_wheel_states,
        args.ui_focus,
#ifndef __ANDROID__
        args.glfw_window,
#endif
        SceneConfigResource{
            .fly = args.arguments.at<bool>(KnownArgs::fly),
            .rotate = args.arguments.at<bool>(KnownArgs::rotate),
            .print_gamepad_buttons = args.arguments.at<bool>(KnownArgs::print_gamepad_buttons),
            .depth_fog = args.arguments.at<bool>(KnownArgs::depth_fog),
            .low_pass = args.arguments.at<bool>(KnownArgs::low_pass),
            .high_pass = args.arguments.at<bool>(KnownArgs::high_pass),
            .with_skybox = args.arguments.at<bool>(KnownArgs::with_skybox),
            .with_flying_logic = args.arguments.at<bool>(KnownArgs::with_flying_logic),
            .background_color = {1.f, 0.f, 1.f},
            .clear_mode = clear_mode_from_string(args.arguments.at<std::string>(KnownArgs::clear_mode))},
        args.script_filename,
        args.arguments.at<size_t>(KnownArgs::max_tracks, 0),
        RaceIdentifier{
            .level = "",
            .session = "",
            .laps = 0,
            .milliseconds = 0},
        [l = args.arguments.try_at(KnownArgs::setup_new_round),
         mle = args.macro_line_executor]()
        {
            if (l.has_value()) {
                mle(JsonView{l.value()}, nullptr, nullptr);
            }
        },
        FocusFilter{
            .focus_mask = focus_from_string(args.arguments.at<std::string>(KnownArgs::focus_mask)),
            .submenu_ids = args.arguments.at_non_null<std::set<std::string>>(KnownArgs::submenus, {})}).second)
    {
        THROW_OR_ABORT("Scene with name \"" + name + "\" already exists");
    }
};
