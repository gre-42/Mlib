#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Text_Logic.hpp>

namespace Mlib {

enum class Focus;
class Focuses;
class AdvanceTimes;

class CountDownLogic:
    public RenderLogic,
    public RenderTextLogic,
    public AdvanceTime,
    public DestructionObserver {
public:
    CountDownLogic(
        AdvanceTimes& advance_times,
        const std::string& ttf_filename,
        const FixedArray<float, 2>& position,
        const ILayoutPixels& font_height,
        const ILayoutPixels& line_distance,
        float duration,
        Focus pending_focus,
        Focus counting_focus,
        std::string text,
        Focuses& focuses);
    ~CountDownLogic();

    // DestructionObserver
    virtual void notify_destroyed(const Object& destroyed_object) override;

    // RenderLogic
    virtual void render(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

    // AdvanceTime
    virtual void advance_time(float dt) override;

private:
    AdvanceTimes& advance_times_;
    float elapsed_time_;
    float duration_;
    Focus pending_focus_;
    Focus counting_focus_;
    std::string text_;
    Focuses& focuses_;
    FixedArray<float, 2> position_;
};

}
