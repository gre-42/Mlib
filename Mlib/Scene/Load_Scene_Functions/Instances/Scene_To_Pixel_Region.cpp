#include "Scene_To_Pixel_Region.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_To_Pixel_Region_Logic.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

using namespace Mlib;

LoadSceneUserFunction SceneToPixelRegion::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*scene_to_pixel_region"
        "\\s+target_scene=([\\w+-.]+)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+size=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+focus_mask=(none|base|menu|loading|countdown_any|scene|game_over|always)"
        "\\s+submenu=(\\w*)$");
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
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    
    std::string target_scene = match[1].str();
    auto wit = args.renderable_scenes.find(target_scene);
    if (wit == args.renderable_scenes.end()) {
        throw std::runtime_error("Could not find renderable scene with name \"" + target_scene + '"');
    }
    std::shared_ptr<RenderToPixelRegionLogic> render_scene_to_pixel_region_logic_;
    render_scene_to_pixel_region_logic_ = std::make_shared<RenderToPixelRegionLogic>(
        render_logics,
        FixedArray<int, 2>{             // position
            safe_stoi(match[2].str()),
            safe_stoi(match[3].str())},
        FixedArray<int, 2>{             // size
            safe_stoi(match[4].str()),
            safe_stoi(match[5].str())},
        FocusFilter{
            .focus_mask = focus_from_string(match[6].str()),
            .submenu_id = match[7].str()});
    RenderingContextGuard rcg{ RenderingContext {.rendering_resources = secondary_rendering_context.rendering_resources, .z_order = 1} };
    wit->second->render_logics_.append(nullptr, render_scene_to_pixel_region_logic_);

}
