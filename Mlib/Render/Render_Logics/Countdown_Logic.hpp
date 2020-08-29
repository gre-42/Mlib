#pragma once
#include <Mlib/Render/Render_Logics/Render_Text_Logic.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <chrono>
#include <memory>

namespace Mlib {

class CountDownLogic: public RenderTextLogic {
public:
    CountDownLogic(
        const std::string& ttf_filename,
        const FixedArray<float, 2>& position,
        float font_height_pixels,
        float line_distance_pixels,
        Focus& focus,
        float nseconds);
    ~CountDownLogic();

    virtual void initialize(GLFWwindow* window) override;
    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual float near_plane() const override;
    virtual float far_plane() const override;
    virtual const FixedArray<float, 4, 4>& vp() const override;
    virtual bool requires_postprocessing() const override;

private:
    std::chrono::time_point<std::chrono::steady_clock> start_time_;
    bool timeout_started_;
    float nseconds_;
    Focus& focus_;
};

}
