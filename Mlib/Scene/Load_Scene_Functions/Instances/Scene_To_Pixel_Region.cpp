#include "Scene_To_Pixel_Region.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_To_Pixel_Region_Logic.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(TARGET_SCENE);
DECLARE_OPTION(POSITION_X);
DECLARE_OPTION(POSITION_Y);
DECLARE_OPTION(SIZE_X);
DECLARE_OPTION(SIZE_Y);
DECLARE_OPTION(FOCUS_MASK);
DECLARE_OPTION(SUBMENUS);

LoadSceneUserFunction SceneToPixelRegion::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*scene_to_pixel_region"
        "\\s+target_scene=([\\w+-.]+)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+size=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+focus_mask=(none|base|menu|loading|countdown_any|scene|game_over|always)"
        "\\s+submenus=(.*)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        SceneToPixelRegion(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

SceneToPixelRegion::SceneToPixelRegion(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SceneToPixelRegion::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::string target_scene = match[TARGET_SCENE].str();
    auto wit = args.renderable_scenes.find(target_scene);
    if (wit == args.renderable_scenes.end()) {
        throw std::runtime_error("Could not find renderable scene with name \"" + target_scene + '"');
    }
    std::shared_ptr<RenderToPixelRegionLogic> render_scene_to_pixel_region_logic_;
    render_scene_to_pixel_region_logic_ = std::make_shared<RenderToPixelRegionLogic>(
        render_logics,
        FixedArray<int, 2>{
            safe_stoi(match[POSITION_X].str()),
            safe_stoi(match[POSITION_Y].str())},
        FixedArray<int, 2>{
            safe_stoi(match[SIZE_X].str()),
            safe_stoi(match[SIZE_Y].str())},
        FocusFilter{
            .focus_mask = focus_from_string(match[FOCUS_MASK].str()),
            .submenu_ids = string_to_set(match[SUBMENUS].str())});
    RenderingContextGuard rcg{ RenderingContext {.rendering_resources = secondary_rendering_context.rendering_resources, .z_order = 1} };
    wit->second->render_logics_.append(nullptr, render_scene_to_pixel_region_logic_);
}
