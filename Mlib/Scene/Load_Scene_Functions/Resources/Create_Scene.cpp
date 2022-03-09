#include "Create_Scene.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Aggregate_Array_Renderer.hpp>
#include <Mlib/Render/Array_Instances_Renderer.hpp>
#include <Mlib/Render/Render_Logics/Clear_Mode.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Aggregate_Renderer.hpp>
#include <Mlib/Scene_Graph/Instances_Renderer.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;

LoadSceneUserFunction CreateScene::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*create_scene"
        "\\s+name=([\\w+-.]+)"
        "\\s+z_order=([\\d-]+)"
        "\\s+fly=(0|1)"
        "\\s+rotate=(0|1)"
        "\\s+print_gamepad_buttons=(0|1)"
        "\\s+depth_fog=(0|1)"
        "\\s+low_pass=(0|1)"
        "\\s+high_pass=(0|1)"
        "\\s+vfx=(0|1)"
        "\\s+with_dirtmap=(0|1)"
        "\\s+with_skybox=(0|1)"
        "\\s+with_flying_logic=(0|1)"
        "\\s+with_pod_bot=(0|1)"
        "\\s+clear_mode=(off|color|depth|color_and_depth)"
        "\\s+max_tracks=(\\d+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void CreateScene::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    RenderingContextGuard rrg{
        args.scene_node_resources,
        match[1].str() + ".rendering_resources",
        args.scene_config.render_config.anisotropic_filtering_level,
        safe_stoi(match[2].str())};
    AggregateRendererGuard arg{std::make_shared<AggregateArrayRenderer>()};
    InstancesRendererGuard irg{std::make_shared<ArrayInstancesRenderer>()};
    auto rs = std::make_shared<RenderableScene>(
        args.scene_node_resources,
        args.scene_config,
        args.button_states,
        args.cursor_states,
        args.scroll_wheel_states,
        args.ui_focus,
        args.window,
        SceneConfigResource{
            .fly = safe_stob(match[3].str()),
            .rotate = safe_stob(match[4].str()),
            .print_gamepad_buttons = safe_stob(match[5].str()),
            .depth_fog = safe_stob(match[6].str()),
            .low_pass = safe_stob(match[7].str()),
            .high_pass = safe_stob(match[8].str()),
            .vfx = safe_stob(match[9].str()),
            .with_dirtmap = safe_stob(match[10].str()),
            .with_skybox = safe_stob(match[11].str()),
            .with_flying_logic = safe_stob(match[12].str()),
            .with_pod_bot = safe_stob(match[13].str()),
            .clear_mode = clear_mode_from_string(match[14].str())},
        args.script_filename,
        safe_stoz(match[15].str()));
    if (!args.renderable_scenes.insert({match[1].str(), rs}).second) {
        throw std::runtime_error("Scene with name \"" + match[1].str() + "\" already exists");
    }
}
