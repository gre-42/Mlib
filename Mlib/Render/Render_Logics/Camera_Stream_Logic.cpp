#include "Camera_Stream_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Render/Render_Logics/Standard_Camera_Logic.hpp>
#include <Mlib/Render/Render_Logics/Standard_Render_Logic.hpp>
#include <Mlib/Render/Render_Setup.hpp>

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

std::optional<RenderSetup> CameraStreamLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return standard_render_logic_->render_setup(lx, ly, frame_id);
}

bool CameraStreamLogic::render_optional_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id,
    const RenderSetup* setup)
{
    LOG_FUNCTION("CameraStreamLogic::render");
    standard_render_logic_->render_auto_setup(
        lx,
        ly,
        render_config,
        scene_graph_config,
        render_results,
        frame_id,
        setup);
    return true;
}

void CameraStreamLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "CameraStreamLogic\n";
    standard_render_logic_->print(ostr, depth + 1);
}

void CameraStreamLogic::set_background_color(const FixedArray<float, 3>& color) {
    standard_render_logic_->set_background_color(color);
}
