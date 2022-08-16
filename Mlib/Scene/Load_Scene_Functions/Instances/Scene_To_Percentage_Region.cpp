#include "Scene_To_Percentage_Region.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_To_Percentage_Region_Logic.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Scene/Renderable_Scenes.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(TARGET_SCENE);
DECLARE_OPTION(Z_ORDER);
DECLARE_OPTION(POSITION_X);
DECLARE_OPTION(POSITION_Y);
DECLARE_OPTION(SIZE_X);
DECLARE_OPTION(SIZE_Y);
DECLARE_OPTION(FOCUS_MASK);
DECLARE_OPTION(SUBMENUS);

LoadSceneUserFunction SceneToPercentageRegion::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*scene_to_percentage_region"
        "\\s+target_scene=([\\w+-.]+)"
        "\\s+z_order=(\\d+)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+size=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+focus_mask=(none|base|menu|loading|countdown_any|scene|game_over|always)"
        "\\s+submenus=(.*)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        SceneToPercentageRegion(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

SceneToPercentageRegion::SceneToPercentageRegion(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SceneToPercentageRegion::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::string target_scene = match[TARGET_SCENE].str();
    auto& rs = args.renderable_scenes[target_scene];
    std::shared_ptr<RenderToPercentageRegionLogic> render_scene_to_pixel_region_logic_;
    render_scene_to_pixel_region_logic_ = std::make_shared<RenderToPercentageRegionLogic>(
        render_logics,
        FixedArray<float, 2>{
            safe_stof(match[POSITION_X].str()),
            safe_stof(match[POSITION_Y].str())},
        FixedArray<float, 2>{
            safe_stof(match[SIZE_X].str()),
            safe_stof(match[SIZE_Y].str())},
        FocusFilter{
            .focus_mask = focus_from_string(match[FOCUS_MASK].str()),
            .submenu_ids = string_to_set(match[SUBMENUS].str())});
    RenderingContextGuard rcg{
        RenderingContext {
            .rendering_resources = secondary_rendering_context.rendering_resources,
            .z_order = safe_stoi(match[Z_ORDER].str())
        }};
    rs.render_logics_.append(nullptr, render_scene_to_pixel_region_logic_);
}
