#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Scene/Render_Logics/Hud_Tracker.hpp>
#include <mutex>
#include <optional>
#include <vector>

namespace Mlib {

class AdvanceTimes;
class Player;
class Players;
enum class ResourceUpdateCycle;
class RenderLogics;
class ObjectPool;

class HudOpponentTrackerLogic:
    public RenderLogic,
    public IAdvanceTime
{
public:
    HudOpponentTrackerLogic(
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
        HudErrorBehavior hud_error_behavior);
    ~HudOpponentTrackerLogic();

    // IAdvanceTime
    virtual void advance_time(float dt, const StaticWorld& world) override;

    // RenderLogic
    virtual std::optional<RenderSetup> try_render_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderedSceneDescriptor& frame_id) const override;
    virtual void render_with_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id,
        const RenderSetup& setup) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    Players& players_;
    DanglingBaseClassRef<Player> player_;
    RenderLogic& scene_logic_;
    HudTracker hud_tracker_;
    DestructionFunctionsRemovalTokens on_player_delete_vehicle_internals_;

    RenderLogics& render_logics_;
};

}
