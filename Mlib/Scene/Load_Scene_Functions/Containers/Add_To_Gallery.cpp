#include "Add_To_Gallery.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logic_Gallery.hpp>
#include <Mlib/Render/Render_Logics/Fill_Pixel_Region_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(RESOURCE);
DECLARE_OPTION(INSTANCE);

LoadSceneUserFunction AddToGallery::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*add_to_gallery"
        "\\s+resource=(#?[\\w+-.\\(\\)/]+)"
        "\\s+instance=([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void AddToGallery::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    args.gallery.insert(
        match[INSTANCE].str(),
        std::make_unique<FillPixelRegionWithTextureLogic>(
            args.fpath(match[RESOURCE].str()).path,
            std::make_unique<Widget>(
                args.layout_constraints.get_pixels("min"),
                args.layout_constraints.get_pixels("max"),
                args.layout_constraints.get_pixels("min"),
                args.layout_constraints.get_pixels("max")),
            ResourceUpdateCycle::ONCE,
            FocusFilter{ .focus_mask = Focus::ALWAYS }));
}
