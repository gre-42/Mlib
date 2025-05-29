#include "Hud_Opponent_Zoom_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Geometry/Cameras/Perspective_Camera.hpp>
#include <Mlib/Geometry/Coordinates/Gl_Look_At_Bounding_Sphere.hpp>
#include <Mlib/Geometry/Intersection/Extremal_Bounding_Sphere.hpp>
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <sstream>

using namespace Mlib;

HudOpponentZoomLogic::HudOpponentZoomLogic(
    ObjectPool& object_pool,
    Scene& scene,
    std::unique_ptr<RenderLogic>&& scene_logic,
    RenderLogics& render_logics,
    Players& players,
    const DanglingBaseClassRef<Player>& player,
    DanglingPtr<SceneNode> exclusive_node,
    std::unique_ptr<IWidget>&& widget,
    float fov,
    float zoom)
    : scene_{ scene }
    , players_{ players }
    , player_{ player }
    , on_player_delete_vehicle_internals_{ player->delete_vehicle_internals, CURRENT_SOURCE_LOCATION }
    , on_clear_exclusive_node_{ exclusive_node == nullptr ? nullptr : &exclusive_node->on_clear, CURRENT_SOURCE_LOCATION }
    , scene_logic_{ std::move(scene_logic) }
    , exclusive_node_{ exclusive_node }
    , widget_{ std::move(widget) }
    , fov_{ fov }
    , scaled_fov_{ std::atan(std::tan(fov_) / zoom) }
{
    render_logics.append({ *this, CURRENT_SOURCE_LOCATION }, 0 /* z_order */, CURRENT_SOURCE_LOCATION);
    if (exclusive_node_ != nullptr) {
        on_clear_exclusive_node_.add([this, &object_pool]() { object_pool.remove(*this); }, CURRENT_SOURCE_LOCATION);
    }
    on_player_delete_vehicle_internals_.add([this, &object_pool]() { object_pool.remove(*this); }, CURRENT_SOURCE_LOCATION);
}

HudOpponentZoomLogic::~HudOpponentZoomLogic() {
    on_destroy.clear();
}

std::optional<RenderSetup> HudOpponentZoomLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return std::nullopt;
}

void HudOpponentZoomLogic::render_without_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("HudOpponentZoomLogic::render");
    if (!player_->has_scene_vehicle()) {
        return;
    }
    auto observer_node = player_->scene_node();
    auto observed_node = player_->target_scene_node();
    if (observed_node == nullptr) {
        return;
    }
    auto ew = widget_->evaluate(lx, ly, YOrientation::AS_IS, RegionRoundMode::ENABLED);
    auto vg = ViewportGuard::from_widget(*ew);
    if (!vg.has_value()) {
        return;
    }
    auto rel_sphere = observed_node->relative_bounding_sphere();
    if (rel_sphere.empty() || rel_sphere.full()) {
        return;
    }
    auto abs_sphere = rel_sphere.data().transformed(observed_node->absolute_model_matrix());
    auto la = gl_lookat_bounding_sphere(
        fov_,
        observer_node->absolute_model_matrix(),
        abs_sphere);
    if (!la.has_value()) {
        return;
    }
    auto zoom_camera_node = make_unique_scene_node(
        la->camera_model_matrix.t,
        matrix_2_tait_bryan_angles(la->camera_model_matrix.R),
        1.f);
    zoom_camera_node->set_camera(
        std::make_unique<PerspectiveCamera>(
            PerspectiveCameraConfig{
                .y_fov = scaled_fov_,
                .near_plane = la->near_plane,
                .far_plane = la->far_plane },
            PerspectiveCamera::Postprocessing::ENABLED));
    RenderedSceneDescriptor zoom_rsd{
        .external_render_pass = {
            frame_id.external_render_pass.user_id,
            ExternalRenderPassType::ZOOM_NODE,
            frame_id.external_render_pass.time,
            VariableAndHash<std::string>(),
            observed_node,
            zoom_camera_node.get(DP_LOC)
        },
        .time_id = 0};
    scene_logic_->render_toplevel(
        LayoutConstraintParameters::child_x(lx, *ew),
        LayoutConstraintParameters::child_y(ly, *ew),
        render_config,
        scene_graph_config,
        nullptr,    // render_results
        zoom_rsd);
}

void HudOpponentZoomLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "HudOpponentZoomLogic";
}
