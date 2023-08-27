#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Scene_Graph/Elements/Node_Hider.hpp>
#include <Mlib/Signal/Exponential_Smoother.hpp>
#include <mutex>

namespace Mlib {

class AdvanceTimes;
class SceneNode;
enum class ResourceUpdateCycle;
class CollisionQuery;
class YawPitchLookAtNodes;

enum class HudErrorBehavior {
    HIDE,
    CENTER
};

HudErrorBehavior hud_error_behavior_from_string(const std::string& s);

class HudImageLogic: public RenderLogic, public FillWithTextureLogic, public NodeHider, public AdvanceTime {
public:
    HudImageLogic(
        RenderLogic* scene_logic,
        CollisionQuery* collision_query,
        DanglingPtr<SceneNode> gun_node,
        DanglingRef<SceneNode> node_to_hide,
        YawPitchLookAtNodes* ypln,
        AdvanceTimes& advance_times,
        const std::string& image_resource_name,
        ResourceUpdateCycle update_cycle,
        const FixedArray<float, 2>& center,
        const FixedArray<float, 2>& size,
        HudErrorBehavior hud_error_behavior);
    ~HudImageLogic();

    // AdvanceTime
    virtual void advance_time(float dt) override;

    // RenderLogic
    virtual void render(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;

    virtual void print(std::ostream& ostr, size_t depth) const override;

    // NodeHider
    virtual bool node_shall_be_hidden(
        DanglingRef<const SceneNode> camera_node,
        const ExternalRenderPass& external_render_pass) const override;

private:
    RenderLogic* scene_logic_;
    CollisionQuery* collision_query_;
    DanglingPtr<SceneNode> gun_node_;
    DanglingRef<SceneNode> node_to_hide_;
    YawPitchLookAtNodes* ypln_;
    AdvanceTimes& advance_times_;
    FixedArray<float, 2> center_;
    FixedArray<float, 2> size_;
    HudErrorBehavior hud_error_behavior_;
    mutable bool is_visible_;
    FixedArray<float, 2> offset_;
    ExponentialSmoother<FixedArray<float, 2>, float> smooth_offset_;
    std::mutex offset_mutex_;
    mutable FixedArray<double, 4, 4> vp_;
    mutable float near_plane_;
    mutable float far_plane_;
    mutable std::mutex render_mutex_;
};

}
