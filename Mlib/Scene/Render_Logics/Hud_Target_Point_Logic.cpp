#include "Hud_Target_Point_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
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
    ObjectPool& object_pool,
    RenderLogic& scene_logic,
    RenderLogics& render_logics,
    const DanglingBaseClassRef<Player>& player,
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
    : object_pool_{ object_pool }
    , collision_query_{ collision_query }
    , gun_node_{ gun_node }
    , ypln_{ ypln }
    , hud_tracker_{
        scene_logic,
        exclusive_node,
        hud_error_behavior,
        center,
        size,
        image_resource_name,
        update_cycle }
    , on_player_delete_externals_{ player->delete_externals, CURRENT_SOURCE_LOCATION }
    , on_destroy_gun_node_{ gun_node->on_destroy, CURRENT_SOURCE_LOCATION }
    , on_clear_exclusive_node_{ exclusive_node == nullptr ? nullptr : &exclusive_node->on_clear, CURRENT_SOURCE_LOCATION }
    , render_logics_{ render_logics }
{
    render_logics_.append({ *this, CURRENT_SOURCE_LOCATION }, 0 /* z_order */, CURRENT_SOURCE_LOCATION);
    on_player_delete_externals_.add([this]() { object_pool_.remove(this); }, CURRENT_SOURCE_LOCATION);
    if (exclusive_node != nullptr) {
        on_clear_exclusive_node_.add([this, &object_pool]() { object_pool.remove(this); }, CURRENT_SOURCE_LOCATION);
    }
    on_destroy_gun_node_.add([this, &object_pool]() { object_pool.remove(this); }, CURRENT_SOURCE_LOCATION);
    advance_times.add_advance_time({ *this, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
}

HudTargetPointLogic::~HudTargetPointLogic() {
    on_destroy.clear();
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
