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
class SceneNode;
class CollisionQuery;
class YawPitchLookAtNodes;
class Scene;
class RenderLogics;
class ObjectPool;

class HudTargetPointLogic:
    public RenderLogic,
    public IAdvanceTime
{
public:
    HudTargetPointLogic(
        ObjectPool& object_pool,
        RenderLogic& scene_logic,
        RenderLogics& render_logics,
        const DanglingBaseClassRef<Player>& player,
        CollisionQuery& collision_query,
        DanglingRef<SceneNode> gun_node,
        DanglingPtr<SceneNode> exclusive_node,
        YawPitchLookAtNodes* ypln,
        AdvanceTimes& advance_times,
        const std::shared_ptr<ITextureHandle>& texture,
        const FixedArray<float, 2>& center,
        const FixedArray<float, 2>& size,
        HudErrorBehavior hud_error_behavior);
    ~HudTargetPointLogic();

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
    ObjectPool& object_pool_;
    CollisionQuery& collision_query_;
    DanglingRef<SceneNode> gun_node_;
    YawPitchLookAtNodes* ypln_;
    RenderLogic& scene_logic_;
    HudTracker hud_tracker_;
    DestructionFunctionsRemovalTokens on_player_delete_vehicle_internals_;
    DestructionFunctionsRemovalTokens on_destroy_gun_node_;
    DestructionFunctionsRemovalTokens on_clear_exclusive_node_;

    RenderLogics& render_logics_;
};

}
