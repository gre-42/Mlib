#include "Moving_Node_Logic.hpp"
#include <Mlib/Geometry/Cameras/Ortho_Camera.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Transformation/Bijection.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

MovingNodeLogic::MovingNodeLogic(DanglingBaseClassRef<SceneNode> skidmark_node)
    : on_skidmark_node_clear{ skidmark_node->on_clear, CURRENT_SOURCE_LOCATION }
    , skidmark_node_{ skidmark_node }
{}

MovingNodeLogic::~MovingNodeLogic() {
    on_destroy.clear();
}

std::optional<RenderSetup> MovingNodeLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return std::nullopt;
}

void MovingNodeLogic::render_without_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("MovingNodeLogic::render");
    auto camera = skidmark_node_->get_camera(CURRENT_SOURCE_LOCATION);
    const auto* ortho_camera = dynamic_cast<OrthoCamera*>(&camera.get());
    if (ortho_camera == nullptr) {
        THROW_OR_ABORT("Moving node camera is not an ortho-camera");
    }
    auto p = ortho_camera->projection_matrix();
    auto bi = skidmark_node_->absolute_bijection(std::chrono::steady_clock::time_point());
    auto vp = dot2d(p.casted<ScenePos>(), bi.view.affine());
    std::optional<FixedArray<float, 2>> offset;
    if (old_camera_position_.has_value()) {
        auto dpi = ortho_camera->dpi(fixed_ones<float, 2>());
        auto diff = bi.view.rotate((*old_camera_position_ - bi.model.t).casted<float>());
        offset = {
            diff(0) * dpi(0),
            diff(1) * dpi(1)
        };
    }
    render_moving_node(
        lx, ly, render_config, scene_graph_config,
        render_results, frame_id, bi, vp, offset);
    old_camera_position_ = bi.model.t;
}
