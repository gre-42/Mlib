#pragma once
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Text_Logic.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <chrono>
#include <memory>

namespace Mlib {

class CountDownLogic: public RenderLogic, public RenderTextLogic {
public:
    CountDownLogic(
        const std::string& ttf_filename,
        const FixedArray<float, 2>& position,
        float font_height_pixels,
        float line_distance_pixels,
        std::list<Focus>& focus,
        float nseconds);
    ~CountDownLogic();

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;

private:
    std::chrono::time_point<std::chrono::steady_clock> start_time_;
    bool timeout_started_;
    float nseconds_;
    std::list<Focus>& focus_;
};

}
