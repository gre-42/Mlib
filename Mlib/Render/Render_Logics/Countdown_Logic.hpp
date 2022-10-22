#pragma once
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Text_Logic.hpp>
#include <chrono>
#include <memory>

namespace Mlib {

enum class Focus;
class Focuses;

class CountDownLogic: public RenderLogic, public RenderTextLogic {
public:
    CountDownLogic(
        const std::string& ttf_filename,
        const FixedArray<float, 2>& position,
        float font_height_pixels,
        float line_distance_pixels,
        float nseconds,
        Focus pending_focus,
        Focus counting_focus,
        const std::string& text,
        Focuses& focuses);
    ~CountDownLogic();

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    std::chrono::duration<float> elapsed_time_;
    float nseconds_;
    Focus pending_focus_;
    Focus counting_focus_;
    std::string text_;
    Focuses& focuses_;
};

}
