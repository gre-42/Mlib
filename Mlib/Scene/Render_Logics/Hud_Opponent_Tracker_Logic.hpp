#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Scene/Render_Logics/Hud_Tracker.hpp>
#include <mutex>

namespace Mlib {

class AdvanceTimes;
class Player;
class Players;
enum class ResourceUpdateCycle;
class RenderLogics;

class HudOpponentTrackerLogic:
    public RenderLogic,
    public IAdvanceTime,
    public std::enable_shared_from_this<HudOpponentTrackerLogic>
{
public:
    HudOpponentTrackerLogic(
        RenderLogic& scene_logic,
        RenderLogics& render_logics,
        Players& players,
        Player& player,
        DanglingPtr<SceneNode> exclusive_node,
        AdvanceTimes& advance_times,
        const std::string& image_resource_name,
        ResourceUpdateCycle update_cycle,
        const FixedArray<float, 2>& center,
        const FixedArray<float, 2>& size,
        HudErrorBehavior hud_error_behavior);
    void init();
    ~HudOpponentTrackerLogic();

    // IAdvanceTime
    virtual void advance_time(float dt, std::chrono::steady_clock::time_point time) override;

    // RenderLogic
    virtual void render(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;

    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    Players& players_;
    Player& player_;
    AdvanceTimes& advance_times_;
    HudTracker hud_tracker_;
    DestructionFunctionsRemovalTokens on_player_delete_externals_;
    DestructionFunctionsRemovalTokens on_clear_exclusive_node_;
    bool shutting_down_;

    RenderLogics& render_logics_;
    DanglingPtr<SceneNode> exclusive_node_;
};

}
