#include "Loading_Text_Logic.hpp"
#include <Mlib/Render/Text/Renderable_Text.hpp>

using namespace Mlib;

LoadingTextLogic::LoadingTextLogic(
    const std::string& ttf_filename,
    const FixedArray<float, 2>& position,
    float font_height_pixels,
    float line_distance_pixels,
    const std::list<Focus>& focus,
    const std::string& text)
: RenderTextLogic{
    ttf_filename,
    position,
    font_height_pixels,
    line_distance_pixels},
  focus_{focus},
  text_{text}
{}

LoadingTextLogic::~LoadingTextLogic()
{}

void LoadingTextLogic::render(
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
        width,
        height,
        line_distance_pixels_,
        true);  // true=periodic_position
}

Focus LoadingTextLogic::focus_mask() const {
    return Focus::LOADING;
}
