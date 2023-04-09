#include "Ui_Exhibit.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logic_Gallery.hpp>
#include <Mlib/Render/Render_Logics/Fill_Pixel_Region_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(Z_ORDER);
DECLARE_OPTION(ID_IN_GALLERY);
DECLARE_OPTION(LEFT);
DECLARE_OPTION(RIGHT);
DECLARE_OPTION(BOTTOM);
DECLARE_OPTION(TOP);
DECLARE_OPTION(UPDATE);
DECLARE_OPTION(FOCUS_MASK);
DECLARE_OPTION(SUBMENUS);

const std::string UiExhibit::key = "ui_exhibit";

LoadSceneUserFunction UiExhibit::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^z_order=(\\d+)"
        "\\s+id_in_gallery=(\\w+)"
        "\\s+left=(\\w+)"
        "\\s+right=(\\w+)"
        "\\s+bottom=(\\w+)"
        "\\s+top=(\\w+)"
        "\\s+update=(\\w+)"
        "\\s+focus_mask=([\\w|]+)"
        "\\s+submenus=(.*)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    UiExhibit(args.renderable_scene()).execute(match, args);
};

UiExhibit::UiExhibit(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void UiExhibit::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    RenderingContextGuard rcg{ RenderingContext{
        .scene_node_resources = primary_rendering_context.scene_node_resources,  // read by FillPixelRegionWithTextureLogic/FillWithTextureLogic
        .rendering_resources = primary_rendering_context.rendering_resources,    // read by FillPixelRegionWithTextureLogic/FillWithTextureLogic
        .z_order = safe_stoi(match[Z_ORDER].str())} };                           // read by RenderLogics
    auto bg = std::make_shared<FillPixelRegionWithTextureLogic>(
        args.gallery[match[ID_IN_GALLERY].str()],
        std::make_unique<Widget>(
            args.layout_constraints.get_pixels(match[LEFT].str()),
            args.layout_constraints.get_pixels(match[RIGHT].str()),
            args.layout_constraints.get_pixels(match[BOTTOM].str()),
            args.layout_constraints.get_pixels(match[TOP].str())),
        FocusFilter{
            .focus_mask = focus_from_string(match[FOCUS_MASK].str()),
            .submenu_ids = string_to_set(match[SUBMENUS].str())});
    render_logics.append(nullptr, bg);
}
