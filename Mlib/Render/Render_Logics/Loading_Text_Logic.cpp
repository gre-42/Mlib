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
    if (focus_.empty()) {
        return;
    }
    if (focus_.back() == Focus::LOADING) {
        renderable_text().render(
            position_,
            text_,
            width,
            height,
            line_distance_pixels_,
            true);  // true=periodic_position
    }
}

float LoadingTextLogic::near_plane() const {
    throw std::runtime_error("LoadingTextLogic::requires_postprocessing not implemented");
}

float LoadingTextLogic::far_plane() const {
    throw std::runtime_error("LoadingTextLogic::requires_postprocessing not implemented");
}

const FixedArray<float, 4, 4>& LoadingTextLogic::vp() const {
    throw std::runtime_error("LoadingTextLogic::vp not implemented");
}

const TransformationMatrix<float, 3>& LoadingTextLogic::iv() const {
    throw std::runtime_error("LoadingTextLogic::iv not implemented");
}

bool LoadingTextLogic::requires_postprocessing() const {
    throw std::runtime_error("LoadingTextLogic::requires_postprocessing not implemented");
}
