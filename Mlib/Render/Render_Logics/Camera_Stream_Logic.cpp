#include "Camera_Stream_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Render/Render_Logics/Standard_Camera_Logic.hpp>
#include <Mlib/Render/Render_Logics/Standard_Render_Logic.hpp>

using namespace Mlib;

CameraStreamLogic::CameraStreamLogic(
    const Scene& scene,
    const SelectedCameras& selected_cameras,
    const FixedArray<float, 3>& background_color,
    ClearMode clear_mode)
    : standard_camera_logic_{ std::make_unique<StandardCameraLogic>(scene, selected_cameras) }
    , standard_render_logic_{ std::make_unique<StandardRenderLogic>(scene, *standard_camera_logic_, background_color, clear_mode) }
{}

CameraStreamLogic::~CameraStreamLogic() {
    on_destroy.clear();
}

void CameraStreamLogic::init(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id)
{
    standard_render_logic_->init(lx, ly, frame_id);
}

void CameraStreamLogic::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("CameraStreamLogic::render");

    standard_render_logic_->render(lx, ly, render_config, scene_graph_config, render_results, frame_id);
}

void CameraStreamLogic::reset() {
    standard_render_logic_->reset();
}

float CameraStreamLogic::near_plane() const {
    return standard_render_logic_->near_plane();
}

float CameraStreamLogic::far_plane() const {
    return standard_render_logic_->far_plane();
}

const FixedArray<ScenePos, 4, 4>& CameraStreamLogic::vp() const {
    return standard_render_logic_->vp();
}

const TransformationMatrix<float, ScenePos, 3>& CameraStreamLogic::iv() const {
    return standard_render_logic_->iv();
}

DanglingPtr<const SceneNode> CameraStreamLogic::camera_node() const {
    return standard_render_logic_->camera_node();
}

bool CameraStreamLogic::requires_postprocessing() const {
    return standard_render_logic_->requires_postprocessing();
}

void CameraStreamLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "CameraStreamLogic\n";
    standard_render_logic_->print(ostr, depth + 1);
}

void CameraStreamLogic::set_background_color(const FixedArray<float, 3>& color) {
    standard_render_logic_->set_background_color(color);
}
