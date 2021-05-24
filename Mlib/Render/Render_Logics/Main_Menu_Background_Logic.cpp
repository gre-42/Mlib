#include "Main_Menu_Background_Logic.hpp"
#include <Mlib/Render/CHK.hpp>
#include <sstream>

using namespace Mlib;

MainMenuBackgroundLogic::MainMenuBackgroundLogic(
    const std::string& image_resource_name,
    ResourceUpdateCycle update_cycle,
    const FocusFilter& focus_filter)
: FillWithTextureLogic{image_resource_name, update_cycle},
  focus_filter_{focus_filter}
{}

void MainMenuBackgroundLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    FillWithTextureLogic::render(width, height, render_config, scene_graph_config, render_results, frame_id);
}

FocusFilter MainMenuBackgroundLogic::focus_filter() const {
    return focus_filter_;
}
