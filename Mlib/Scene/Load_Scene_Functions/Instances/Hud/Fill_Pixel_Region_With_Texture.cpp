#include "Fill_Pixel_Region_With_Texture.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Fill_Pixel_Region_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
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
DECLARE_OPTION(SOURCE_SCENE);
DECLARE_OPTION(TEXTURE_NAME);
DECLARE_OPTION(UPDATE);
DECLARE_OPTION(POSITION_X);
DECLARE_OPTION(POSITION_Y);
DECLARE_OPTION(SIZE_X);
DECLARE_OPTION(SIZE_Y);
DECLARE_OPTION(FOCUS_MASK);
DECLARE_OPTION(SUBMENUS);

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
        "\\s+submenus=(.*)$");
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
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::string source_scene = match[SOURCE_SCENE].str();
    auto& rs = args.renderable_scenes[source_scene];
    std::shared_ptr<FillPixelRegionWithTextureLogic> scene_window_logic;
    {
        RenderingContextGuard rcg{rs.secondary_rendering_context_};
        scene_window_logic = std::make_shared<FillPixelRegionWithTextureLogic>(
            match[TEXTURE_NAME].str(),
            resource_update_cycle_from_string(match[UPDATE].str()),
            FixedArray<float, 2>{
                safe_stof(match[POSITION_X].str()),
                safe_stof(match[POSITION_Y].str())},
            FixedArray<float, 2>{
                safe_stof(match[SIZE_X].str()),
                safe_stof(match[SIZE_Y].str())},
            FocusFilter{
                .focus_mask = focus_from_string(match[FOCUS_MASK].str()),
                .submenu_ids = string_to_set(match[SUBMENUS].str())});
    }
    render_logics.append(nullptr, scene_window_logic);

}
