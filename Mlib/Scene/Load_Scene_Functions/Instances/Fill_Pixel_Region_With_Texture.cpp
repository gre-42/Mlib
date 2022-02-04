#include "Fill_Pixel_Region_With_Texture.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Fill_Pixel_Region_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

using namespace Mlib;

LoadSceneUserFunction FillPixelRegionWithTexture::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*fill_pixel_region_with_texture"
        "\\s+source_scene=([\\w+-.]+)"
        "\\s+texture_name=([\\w+-.]+)"
        "\\s+update=(once|always)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+size=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+focus_mask=(none|base|menu|loading|countdown_any|scene|game_over|always)"
        "\\s+submenu=(\\w*)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        FillPixelRegionWithTexture(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

FillPixelRegionWithTexture::FillPixelRegionWithTexture(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void FillPixelRegionWithTexture::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::string source_scene = match[1].str();
    auto wit = args.renderable_scenes.find(source_scene);
    if (wit == args.renderable_scenes.end()) {
        throw std::runtime_error("Could not find renderable scene with name \"" + source_scene + '"');
    }
    std::shared_ptr<FillPixelRegionWithTextureLogic> scene_window_logic;
    {
        RenderingContextGuard rcg{wit->second->secondary_rendering_context_};
        scene_window_logic = std::make_shared<FillPixelRegionWithTextureLogic>(
            match[2].str(),                   // texture name
            resource_update_cycle_from_string(match[3].str()),
            FixedArray<float, 2>{             // position
                safe_stof(match[4].str()),
                safe_stof(match[5].str())},
            FixedArray<float, 2>{             // size
                safe_stof(match[6].str()),
                safe_stof(match[7].str())},
            FocusFilter{
                .focus_mask = focus_from_string(match[8].str()),
                .submenu_id = match[9].str()});
    }
    render_logics.append(nullptr, scene_window_logic);

}
