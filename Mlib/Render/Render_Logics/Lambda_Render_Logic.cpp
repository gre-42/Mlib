#include "Lambda_Render_Logic.hpp"

using namespace Mlib;

LambdaRenderLogic::LambdaRenderLogic(
    RenderLogic& render_logic,
    const std::function<void()>& lambda)
: render_logic_{ render_logic },
  lambda_{ lambda }
{}

LambdaRenderLogic::~LambdaRenderLogic()
{}

void LambdaRenderLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    render_logic_.render(
        width,
        height,
        render_config,
        scene_graph_config,
        render_results,
        frame_id);
    lambda_();
}
