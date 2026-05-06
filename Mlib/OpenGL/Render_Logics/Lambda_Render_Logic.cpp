#include "Lambda_Render_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Misc/Log.hpp>
#include <Mlib/OpenGL/Render_Setup.hpp>
#include <stdexcept>

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
    throw std::runtime_error("Print not supported by LambdaRenderLogic");
}
