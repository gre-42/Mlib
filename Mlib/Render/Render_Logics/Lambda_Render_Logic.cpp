#include "Lambda_Render_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

LambdaRenderLogic::LambdaRenderLogic(const Lambda& lambda)
: lambda_{ lambda }
{}

LambdaRenderLogic::~LambdaRenderLogic()
{}

void LambdaRenderLogic::render(
    int width,
    int height,
    float xdpi,
    float ydpi,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("LambdaRenderLogic::render");
    lambda_(
        width,
        height,
        xdpi,
        ydpi,
        render_config,
        scene_graph_config,
        render_results,
        frame_id);
}

void LambdaRenderLogic::print(std::ostream& ostr, size_t depth) const {
    THROW_OR_ABORT("Print not supported by LambdaRenderLogic");
}
