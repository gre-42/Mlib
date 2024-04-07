#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Scene/Render_Logics/Hud_Tracker.hpp>
#include <mutex>

namespace Mlib {

class AdvanceTimes;
class SceneNode;
enum class ResourceUpdateCycle;
class CollisionQuery;
class YawPitchLookAtNodes;

class HudTargetPointLogic: public RenderLogic, public IAdvanceTime {
public:
    HudTargetPointLogic(
        RenderLogic* scene_logic,
        CollisionQuery* collision_query,
        DanglingPtr<SceneNode> gun_node,
        DanglingPtr<SceneNode> exclusive_node,
        YawPitchLookAtNodes* ypln,
        AdvanceTimes& advance_times,
        const std::string& image_resource_name,
        ResourceUpdateCycle update_cycle,
        const FixedArray<float, 2>& center,
        const FixedArray<float, 2>& size,
        HudErrorBehavior hud_error_behavior);
    ~HudTargetPointLogic();

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
    CollisionQuery* collision_query_;
    DanglingPtr<SceneNode> gun_node_;
    YawPitchLookAtNodes* ypln_;
    AdvanceTimes& advance_times_;
    HudTracker hud_tracker_;
};

}
