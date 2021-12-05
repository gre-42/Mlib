#include "Focused_Text_Logic.hpp"
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>

using namespace Mlib;

FocusedTextLogic::FocusedTextLogic(
    const std::string& ttf_filename,
    const FixedArray<float, 2>& position,
    float font_height_pixels,
    float line_distance_pixels,
    Focus focus_mask,
    const std::string& text)
: RenderTextLogic{
    ttf_filename,
    position,
    font_height_pixels,
    line_distance_pixels},
  text_{text},
  focus_mask_{focus_mask}
{}

FocusedTextLogic::~FocusedTextLogic()
{}

void FocusedTextLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    renderable_text().render(
        position_,
        text_,
        FixedArray<int, 2>{ width, height },
        line_distance_pixels_,
        true);  // true=periodic_position
}

FocusFilter FocusedTextLogic::focus_filter() const {
    return { .focus_mask = focus_mask_ };
}

void FocusedTextLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "FocusedTextLogic (" << focus_to_string(focus_mask_) << ")\n";
}
