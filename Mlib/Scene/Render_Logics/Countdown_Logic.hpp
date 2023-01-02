#pragma once
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Text_Logic.hpp>

namespace Mlib {

enum class Focus;
class Focuses;

class CountDownLogic: public RenderLogic, public RenderTextLogic, public AdvanceTime {
public:
    CountDownLogic(
        const std::string& ttf_filename,
        const FixedArray<float, 2>& position,
        float font_height_pixels,
        float line_distance_pixels,
        float duration,
        Focus pending_focus,
        Focus counting_focus,
        std::string text,
        Focuses& focuses);
    ~CountDownLogic();

    // RenderLogic
    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

    // AdvanceTime
    virtual void advance_time(float dt) override;

private:
    float elapsed_time_;
    float duration_;
    Focus pending_focus_;
    Focus counting_focus_;
    std::string text_;
    Focuses& focuses_;
};

}
