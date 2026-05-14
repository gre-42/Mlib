#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Scene/Render_Logics/Hud_Tracker.hpp>
#include <mutex>
#include <optional>
#include <vector>
#ifndef WITHOUT_GRAPHICS
#include <Mlib/OpenGL/Render_Logic.hpp>
#endif

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
#ifndef WITHOUT_GRAPHICS
    public RenderLogic,
#else
    public virtual DanglingBaseClass,
#endif
    public IAdvanceTime
{
public:
    HudTargetPointLogic(
        ObjectPool& object_pool,
        #ifndef WITHOUT_GRAPHICS
        RenderLogic& scene_logic,
        RenderLogics& render_logics,
        const std::shared_ptr<ITextureHandle>& texture,
        const FixedArray<float, 2>& center,
        const FixedArray<float, 2>& size,
        #endif
        const DanglingBaseClassRef<Player>& player,
        CollisionQuery& collision_query,
        const DanglingBaseClassRef<SceneNode>& gun_node,
        const std::optional<std::vector<DanglingBaseClassPtr<const SceneNode>>>& exclusive_nodes,
        const DanglingBaseClassPtr<YawPitchLookAtNodes>& ypln,
        AdvanceTimes& advance_times,
        HudErrorBehavior hud_error_behavior);
    ~HudTargetPointLogic();

    // IAdvanceTime
    virtual void advance_time(float dt, const StaticWorld& world) override;

    // RenderLogic
    #ifndef WITHOUT_GRAPHICS
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
    #endif

private:
    DanglingBaseClassRef<SceneNode> gun_node_;
    DanglingBaseClassPtr<YawPitchLookAtNodes> ypln_;
    #ifndef WITHOUT_GRAPHICS
    CollisionQuery& collision_query_;
    RenderLogic& scene_logic_;
    HudTracker hud_tracker_;
    #endif
    DestructionFunctionsRemovalTokens on_player_delete_vehicle_internals_;
    DestructionFunctionsRemovalTokens on_destroy_gun_node_;
    DestructionFunctionsRemovalTokens on_destroy_ypln_;
};

}
