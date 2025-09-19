#include "Hud_Opponent_Tracker_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <sstream>

using namespace Mlib;

HudOpponentTrackerLogic::HudOpponentTrackerLogic(
    ObjectPool& object_pool,
    RenderLogic& scene_logic,
    RenderLogics& render_logics,
    Players& players,
    const DanglingBaseClassRef<Player>& player,
    const std::optional<std::vector<DanglingBaseClassPtr<const SceneNode>>>& exclusive_nodes,
    AdvanceTimes& advance_times,
    const std::shared_ptr<ITextureHandle>& texture,
    const FixedArray<float, 2>& center,
    const FixedArray<float, 2>& size,
    HudErrorBehavior hud_error_behavior)
    : players_{ players }
    , player_{ player }
    , scene_logic_{ scene_logic }
    , hud_tracker_{
        exclusive_nodes,
        hud_error_behavior,
        center,
        size,
        texture }
    , on_player_delete_vehicle_internals_{ player->delete_vehicle_internals, CURRENT_SOURCE_LOCATION }
    , render_logics_{ render_logics }
{
    render_logics_.append({ *this, CURRENT_SOURCE_LOCATION }, 0 /* z_order */, CURRENT_SOURCE_LOCATION);
    on_player_delete_vehicle_internals_.add([this, &object_pool]() { object_pool.remove(*this); }, CURRENT_SOURCE_LOCATION);
    advance_times.add_advance_time({ *this, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
}

HudOpponentTrackerLogic::~HudOpponentTrackerLogic() {
    on_destroy.clear();
}

void HudOpponentTrackerLogic::advance_time(float dt, const StaticWorld& world) {
    auto target_rb = player_->target_rb();
    if (target_rb == nullptr) {
        hud_tracker_.invalidate();
        return;
    }
    auto ht = hud_tracker_.time_advancer();
    if (!ht.is_visible()) {
        return;
    }
    ht.advance_time(target_rb->rbp_.abs_position());
}

std::optional<RenderSetup> HudOpponentTrackerLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return scene_logic_.render_setup(lx, ly, frame_id);
}

void HudOpponentTrackerLogic::render_with_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id,
    const RenderSetup& setup)
{
    LOG_FUNCTION("HudOpponentTrackerLogic::render");
    hud_tracker_.render(lx, ly, frame_id, setup);
}

void HudOpponentTrackerLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "HudOpponentTrackerLogic";
}
