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

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NAME);
DECLARE_OPTION(Z_ORDER);
DECLARE_OPTION(FLY);
DECLARE_OPTION(ROTATE);
DECLARE_OPTION(PRINT_GAMEPAD_BUTTONS);
DECLARE_OPTION(DEPTH_FOG);
DECLARE_OPTION(LOW_PASS);
DECLARE_OPTION(HIGH_PASS);
DECLARE_OPTION(VFX);
DECLARE_OPTION(WITH_DIRTMAP);
DECLARE_OPTION(WITH_SKYBOX);
DECLARE_OPTION(WITH_FLYING_LOGIC);
DECLARE_OPTION(WITH_POD_BOT);
DECLARE_OPTION(CLEAR_MODE);
DECLARE_OPTION(MAX_TRACKS);

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
        match[NAME].str() + ".rendering_resources",
        args.scene_config.render_config.anisotropic_filtering_level,
        safe_stoi(match[Z_ORDER].str())};
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
            .fly = safe_stob(match[FLY].str()),
            .rotate = safe_stob(match[ROTATE].str()),
            .print_gamepad_buttons = safe_stob(match[PRINT_GAMEPAD_BUTTONS].str()),
            .depth_fog = safe_stob(match[DEPTH_FOG].str()),
            .low_pass = safe_stob(match[LOW_PASS].str()),
            .high_pass = safe_stob(match[HIGH_PASS].str()),
            .vfx = safe_stob(match[VFX].str()),
            .with_dirtmap = safe_stob(match[WITH_DIRTMAP].str()),
            .with_skybox = safe_stob(match[WITH_SKYBOX].str()),
            .with_flying_logic = safe_stob(match[WITH_FLYING_LOGIC].str()),
            .with_pod_bot = safe_stob(match[WITH_POD_BOT].str()),
            .clear_mode = clear_mode_from_string(match[CLEAR_MODE].str())},
        args.script_filename,
        safe_stoz(match[MAX_TRACKS].str()));
    if (!args.renderable_scenes.insert({match[NAME].str(), rs}).second) {
        throw std::runtime_error("Scene with name \"" + match[NAME].str() + "\" already exists");
    }
}
