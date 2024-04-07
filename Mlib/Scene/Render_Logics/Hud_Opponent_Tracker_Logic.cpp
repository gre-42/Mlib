#include "Hud_Opponent_Tracker_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <sstream>

using namespace Mlib;

HudOpponentTrackerLogic::HudOpponentTrackerLogic(
    RenderLogic* scene_logic,
    Players& players,
    Player& player,
    DanglingPtr<SceneNode> exclusive_node,
    AdvanceTimes& advance_times,
    const std::string& image_resource_name,
    ResourceUpdateCycle update_cycle,
    const FixedArray<float, 2>& center,
    const FixedArray<float, 2>& size,
    HudErrorBehavior hud_error_behavior)
    : players_{ players }
    , player_{ player }
    , advance_times_{ advance_times }
    , hud_tracker_{
        scene_logic,
        exclusive_node,
        hud_error_behavior,
        center,
        size,
        image_resource_name,
        update_cycle }
{
    advance_times_.add_advance_time(*this);
}

HudOpponentTrackerLogic::~HudOpponentTrackerLogic() {
    advance_times_.delete_advance_time(*this, CURRENT_SOURCE_LOCATION);
}

void HudOpponentTrackerLogic::advance_time(float dt, std::chrono::steady_clock::time_point time) {
    auto target_rb = player_.target_rb();
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

void HudOpponentTrackerLogic::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("HudOpponentTrackerLogic::render");
    hud_tracker_.render(lx, ly, frame_id);
}

void HudOpponentTrackerLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "HudOpponentTrackerLogic\n";
}
