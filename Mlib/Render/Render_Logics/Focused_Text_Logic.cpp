#include "Focused_Text_Logic.hpp"
#include <Mlib/Layout/ILayout_Pixels.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Render/Text/Align_Text.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>

using namespace Mlib;

FocusedTextLogic::FocusedTextLogic(
    const std::string& ttf_filename,
    const FixedArray<float, 2>& position,
    const ILayoutPixels& font_height,
    const ILayoutPixels& line_distance,
    Focus focus_mask,
    std::string text)
: RenderTextLogic{
    ttf_filename,
    font_height,
    line_distance},
  position_{position},
  text_{std::move(text)},
  focus_mask_{focus_mask}
{}

FocusedTextLogic::~FocusedTextLogic() = default;

void FocusedTextLogic::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("FocusedTextLogic::render");
    renderable_text().render(
        font_height_.to_pixels(ly),
        position_,
        {lx.flength(), ly.flength()},
        text_,
        AlignText::BOTTOM,
        line_distance_.to_pixels(ly));
}

FocusFilter FocusedTextLogic::focus_filter() const {
    return { .focus_mask = focus_mask_ };
}

void FocusedTextLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "FocusedTextLogic (" << focus_to_string(focus_mask_) << ")\n";
}
