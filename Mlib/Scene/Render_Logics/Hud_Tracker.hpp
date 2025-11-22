#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unordered_Set.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <Mlib/Signal/Exponential_Smoother.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <vector>

namespace Mlib {

class Scene;
class SceneNode;
struct RenderedSceneDescriptor;
struct LayoutConstraintParameters;
class HudTracker;
struct RenderSetup;

enum class HudErrorBehavior {
    HIDE,
    CENTER
};

HudErrorBehavior hud_error_behavior_from_string(const std::string& s);

class HudTrackerTimeAdvancer {
public:
    explicit HudTrackerTimeAdvancer(HudTracker& tracker);
    bool is_visible() const;
    void advance_time(const FixedArray<ScenePos, 3>& point);
private:
    HudTracker& tracker_;
    bool is_visible_;
    FixedArray<ScenePos, 4, 4> vp_;
    float near_plane_;
    float far_plane_;
};

class HudTracker: public FillWithTextureLogic {
    friend HudTrackerTimeAdvancer;
    HudTracker(const HudTracker&) = delete;
    HudTracker& operator = (const HudTracker&) = delete;
public:
    HudTracker(
        const std::optional<std::vector<DanglingBaseClassPtr<const SceneNode>>>& exclusive_nodes,
        HudErrorBehavior hud_error_behavior,
        const FixedArray<float, 2>& center,
        const FixedArray<float, 2>& size,
        const std::shared_ptr<ITextureHandle>& texture);
    ~HudTracker();
    void invalidate();
    HudTrackerTimeAdvancer time_advancer();
    void render(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderedSceneDescriptor& frame_id,
        const RenderSetup& setup);
private:
    mutable FastMutex render_mutex_;
    FastMutex offset_mutex_;
    FixedArray<float, 2> center_;
    FixedArray<float, 2> size_;
    HudErrorBehavior hud_error_behavior_;
    FixedArray<float, 2> offset_;
    ExponentialSmoother<FixedArray<float, 2>, float> smooth_offset_;
    mutable bool is_visible_;
    std::optional<DanglingUnorderedSet<const SceneNode>> exclusive_nodes_;
    mutable FixedArray<ScenePos, 4, 4> vp_;
    mutable float near_plane_;
    mutable float far_plane_;
};

}
