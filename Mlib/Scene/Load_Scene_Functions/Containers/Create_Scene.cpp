#include "Create_Scene.hpp"
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Physics/Containers/Race_Identifier.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Aggregate_Array_Renderer.hpp>
#include <Mlib/Render/Array_Instances_Renderer.hpp>
#include <Mlib/Render/Array_Instances_Renderers.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Logics/Clear_Mode.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Load_Scene_User_Function_Args.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Scene/Renderable_Scenes.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene_Graph/Aggregate_Renderer.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <Mlib/Scene_Graph/Instances_Renderer.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NAME);
DECLARE_OPTION(Z_ORDER);
DECLARE_OPTION(FOCUS_MASK);
DECLARE_OPTION(SUBMENUS);
DECLARE_OPTION(FLY);
DECLARE_OPTION(ROTATE);
DECLARE_OPTION(PRINT_GAMEPAD_BUTTONS);
DECLARE_OPTION(DEPTH_FOG);
DECLARE_OPTION(LOW_PASS);
DECLARE_OPTION(HIGH_PASS);
DECLARE_OPTION(WITH_SKYBOX);
DECLARE_OPTION(WITH_FLYING_LOGIC);
DECLARE_OPTION(CLEAR_MODE);
DECLARE_OPTION(MAX_TRACKS);
DECLARE_OPTION(SETUP_NEW_ROUND);

const std::string CreateScene::key = "create_scene";

LoadSceneUserFunction CreateScene::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^name=([\\w+-.]+)"
        "\\s+z_order=([\\d-]+)"
        "\\s+focus_mask=([\\w|]+)"
        "\\s+submenus=(.*)"
        "\\s+fly=(0|1)"
        "\\s+rotate=(0|1)"
        "\\s+print_gamepad_buttons=(0|1)"
        "\\s+depth_fog=(0|1)"
        "\\s+low_pass=(0|1)"
        "\\s+high_pass=(0|1)"
        "\\s+with_skybox=(0|1)"
        "\\s+with_flying_logic=(0|1)"
        "\\s+clear_mode=(off|color|depth|color_and_depth)"
        "(?:\\s+max_tracks=(\\d+))?"
        "(?:\\s+setup_new_round=([\\S\\s]+))?$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    execute(match, args);
};

void CreateScene::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto rrg = RenderingContextGuard::layer(
        args.scene_node_resources,
        match[NAME].str() + ".rendering_resources",
        args.scene_config.render_config.anisotropic_filtering_level,
        safe_stoi(match[Z_ORDER].str()));
    AggregateRendererGuard arg{
        std::make_shared<AggregateArrayRenderer>(),
        std::make_shared<AggregateArrayRenderer>()};
    InstancesRendererGuard irg{
        std::make_shared<ArrayInstancesRenderers>(),
        std::make_shared<ArrayInstancesRenderer>()};
    std::string setup_new_round =
        match[SETUP_NEW_ROUND].str();
    if (!args.renderable_scenes.try_emplace(
        match[NAME].str(),
        args.scene_node_resources,
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
            .fly = safe_stob(match[FLY].str()),
            .rotate = safe_stob(match[ROTATE].str()),
            .print_gamepad_buttons = safe_stob(match[PRINT_GAMEPAD_BUTTONS].str()),
            .depth_fog = safe_stob(match[DEPTH_FOG].str()),
            .low_pass = safe_stob(match[LOW_PASS].str()),
            .high_pass = safe_stob(match[HIGH_PASS].str()),
            .with_skybox = safe_stob(match[WITH_SKYBOX].str()),
            .with_flying_logic = safe_stob(match[WITH_FLYING_LOGIC].str()),
            .background_color = {1.f, 0.f, 1.f},
            .clear_mode = clear_mode_from_string(match[CLEAR_MODE].str())},
        args.script_filename,
        match[MAX_TRACKS].matched ? safe_stoz(match[MAX_TRACKS].str()) : 0,
        RaceIdentifier{
            .level = "",
            .session = "",
            .laps = 0,
            .milliseconds = 0},
        [setup_new_round,
         mle = args.macro_line_executor]()
        {
            mle(setup_new_round, nullptr);
        },
        FocusFilter{
            .focus_mask = focus_from_string(match[FOCUS_MASK].str()),
            .submenu_ids = string_to_set(match[SUBMENUS].str())}).second)
    {
        THROW_OR_ABORT("Scene with name \"" + match[NAME].str() + "\" already exists");
    }
}
