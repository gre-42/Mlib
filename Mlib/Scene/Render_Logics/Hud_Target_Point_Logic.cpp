#include "Hud_Target_Point_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Pitch_Look_At_Node.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Containers/Collision_Query.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <sstream>

using namespace Mlib;

HudTargetPointLogic::HudTargetPointLogic(
    RenderLogic& scene_logic,
    RenderLogics& render_logics,
    Player& player,
    CollisionQuery& collision_query,
    DanglingRef<SceneNode> gun_node,
    DanglingPtr<SceneNode> exclusive_node,
    YawPitchLookAtNodes* ypln,
    AdvanceTimes& advance_times,
    const std::string& image_resource_name,
    ResourceUpdateCycle update_cycle,
    const FixedArray<float, 2>& center,
    const FixedArray<float, 2>& size,
    HudErrorBehavior hud_error_behavior)
    : collision_query_{ collision_query }
    , gun_node_{ gun_node }
    , ypln_{ ypln }
    , advance_times_{ advance_times }
    , hud_tracker_{
        scene_logic,
        exclusive_node,
        hud_error_behavior,
        center,
        size,
        image_resource_name,
        update_cycle }
    , on_player_delete_externals_{ player.delete_externals }
    , on_clear_exclusive_node_{ exclusive_node == nullptr ? nullptr : &exclusive_node->on_clear }
    , shutting_down_{ false }
    , render_logics_{ render_logics }
    , exclusive_node_{ exclusive_node }
{}

void HudTargetPointLogic::init() {
    render_logics_.append(exclusive_node_, shared_from_this(), 0 /* z_order */);
    if (exclusive_node_ != nullptr) {
        on_clear_exclusive_node_.add([this]() { if (!shutting_down_) { render_logics_.remove(*this); }});
    }
    on_player_delete_externals_.add([this]() { if (!shutting_down_) { render_logics_.remove(*this); }});
    advance_times_.add_advance_time(*this);
}

HudTargetPointLogic::~HudTargetPointLogic() {
    if (shutting_down_) {
        verbose_abort("HudTargetPointLogic already shutting down");
    }
    shutting_down_ = true;
    advance_times_.delete_advance_time(*this, CURRENT_SOURCE_LOCATION);
}

void HudTargetPointLogic::advance_time(float dt, std::chrono::steady_clock::time_point time) {
    if (ypln_ != nullptr) {
        float dpitch_head = ypln_->pitch_look_at_node().get_dpitch_head();
        if (!std::isnan(dpitch_head) && (dpitch_head != 0.f)) {
            hud_tracker_.invalidate();
            return;
        }
    }
    auto ht = hud_tracker_.time_advancer();
    if (!ht.is_visible()) {
        return;
    }
    auto gun_pose = gun_node_->absolute_model_matrix();
    FixedArray<double, 3> intersection_point;
    if (collision_query_.can_see(
        gun_pose.t(),
        gun_pose.t() - 1000.0 * gun_pose.R().column(2).casted<double>(),
        nullptr, // excluded0,
        nullptr, // excluded1
        false, // only_terrain
        PhysicsMaterial::OBJ_BULLET_COLLIDABLE_MASK,
        &intersection_point))
    {
        hud_tracker_.invalidate();
        return;
    }
    ht.advance_time(intersection_point);
}

void HudTargetPointLogic::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("HudTargetPointLogic::render");
    hud_tracker_.render(lx, ly, frame_id);
}

void HudTargetPointLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "HudTargetPointLogic\n";
}
