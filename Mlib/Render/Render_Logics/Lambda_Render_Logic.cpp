#include "Lambda_Render_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

LambdaRenderLogic::LambdaRenderLogic(const Lambda& lambda)
    : lambda_{ lambda }
{}

LambdaRenderLogic::~LambdaRenderLogic() = default;

std::optional<RenderSetup> LambdaRenderLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return std::nullopt;
}

void LambdaRenderLogic::render_without_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("LambdaRenderLogic::render");
    lambda_(
        lx,
        ly,
        render_config,
        scene_graph_config,
        render_results,
        frame_id);
}

void LambdaRenderLogic::print(std::ostream& ostr, size_t depth) const {
    THROW_OR_ABORT("Print not supported by LambdaRenderLogic");
}
