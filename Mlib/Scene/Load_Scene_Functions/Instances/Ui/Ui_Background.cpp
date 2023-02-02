#include "Ui_Background.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Fill_Pixel_Region_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(Z_ORDER);
DECLARE_OPTION(TEXTURE);
DECLARE_OPTION(LEFT);
DECLARE_OPTION(RIGHT);
DECLARE_OPTION(BOTTOM);
DECLARE_OPTION(TOP);
DECLARE_OPTION(UPDATE);
DECLARE_OPTION(FOCUS_MASK);

LoadSceneUserFunction UiBackground::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*ui_background"
        "\\s+z_order=(\\d+)"
        "\\s+texture=([\\w+-. \\(\\)/]+)"
        "\\s+left=(\\w+)"
        "\\s+right=(\\w+)"
        "\\s+bottom=(\\w+)"
        "\\s+top=(\\w+)"
        "\\s+update=(\\w+)"
        "\\s+focus_mask=([\\w|]+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        UiBackground(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

UiBackground::UiBackground(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void UiBackground::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    RenderingContextGuard rcg{ RenderingContext{
        .scene_node_resources = primary_rendering_context.scene_node_resources,  // read by FillPixelRegionWithTextureLogic/FillWithTextureLogic
        .rendering_resources = primary_rendering_context.rendering_resources,    // read by FillPixelRegionWithTextureLogic/FillWithTextureLogic
        .z_order = safe_stoi(match[Z_ORDER].str())} };                           // read by RenderLogics
    auto bg = std::make_shared<FillPixelRegionWithTextureLogic>(
        std::make_shared<FillWithTextureLogic>(
            args.fpath(match[TEXTURE].str()).path,
            resource_update_cycle_from_string(match[UPDATE].str()),
            ColorMode::RGBA),
        std::make_unique<Widget>(
            args.layout_constraints.get_pixels(match[LEFT].str()),
            args.layout_constraints.get_pixels(match[RIGHT].str()),
            args.layout_constraints.get_pixels(match[BOTTOM].str()),
            args.layout_constraints.get_pixels(match[TOP].str())),
        FocusFilter{ .focus_mask = focus_from_string(match[FOCUS_MASK].str()) });
    render_logics.append(nullptr, bg);
}
