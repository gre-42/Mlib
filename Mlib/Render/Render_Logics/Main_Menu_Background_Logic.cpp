#include "Main_Menu_Background_Logic.hpp"
#include <Mlib/Render/CHK.hpp>
#include <sstream>

using namespace Mlib;

MainMenuBackgroundLogic::MainMenuBackgroundLogic(
    RenderingResources& rendering_resources,
    const std::string& image_resource_name,
    const std::list<Focus>& focus,
    Focus target_focus)
: FillWithTextureLogic{rendering_resources, image_resource_name},
  focus_{focus},
  target_focus_{target_focus}
{}

void MainMenuBackgroundLogic::render(
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
    if (focus_.back() == target_focus_) {
        FillWithTextureLogic::render(width, height, render_config, scene_graph_config, render_results, frame_id);
    }
}

float MainMenuBackgroundLogic::near_plane() const {
    throw std::runtime_error("MainMenuBackgroundLogic::requires_postprocessing not implemented");
}

float MainMenuBackgroundLogic::far_plane() const {
    throw std::runtime_error("MainMenuBackgroundLogic::requires_postprocessing not implemented");
}

const FixedArray<float, 4, 4>& MainMenuBackgroundLogic::vp() const {
    throw std::runtime_error("MainMenuBackgroundLogic::vp not implemented");
}

const TransformationMatrix<float>& MainMenuBackgroundLogic::iv() const {
    throw std::runtime_error("MainMenuBackgroundLogic::iv not implemented");
}

bool MainMenuBackgroundLogic::requires_postprocessing() const {
    throw std::runtime_error("MainMenuBackgroundLogic::requires_postprocessing not implemented");
}
