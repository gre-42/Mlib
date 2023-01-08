#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Scene_Graph/Elements/Node_Hider.hpp>
#include <Mlib/Signal/Exponential_Smoother.hpp>
#include <mutex>

namespace Mlib {

class AdvanceTimes;
class SceneNode;
enum class ResourceUpdateCycle;
class RenderLogic;
class CollisionQuery;
class YawPitchLookAtNodes;

class HudImageLogic: public DestructionObserver, public FillWithTextureLogic, public NodeHider, public AdvanceTime {
public:
    HudImageLogic(
        RenderLogic* scene_logic,
        CollisionQuery* collision_query,
        SceneNode* gun_node,
        SceneNode& node_to_hide,
        YawPitchLookAtNodes* ypln,
        AdvanceTimes& advance_times,
        const std::string& image_resource_name,
        ResourceUpdateCycle update_cycle,
        const FixedArray<float, 2>& center,
        const FixedArray<float, 2>& size);

    virtual void notify_destroyed(Object& destroyed_object) override;

    virtual void advance_time(float dt) override;

    virtual void render(
        int width,
        int height,
        float xdpi,
        float ydpi,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;

    virtual bool node_shall_be_hidden(
        const SceneNode& camera_node,
        const ExternalRenderPass& external_render_pass) const override;

private:
    void render(int width, int height);
    RenderLogic* scene_logic_;
    CollisionQuery* collision_query_;
    SceneNode* gun_node_;
    SceneNode& node_to_hide_;
    YawPitchLookAtNodes* ypln_;
    AdvanceTimes& advance_times_;
    FixedArray<float, 2> center_;
    FixedArray<float, 2> size_;
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
