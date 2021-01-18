#pragma once
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Text_Logic.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

namespace Mlib {

class LoadingTextLogic: public RenderLogic, public RenderTextLogic {
public:
    LoadingTextLogic(
        const std::string& ttf_filename,
        const FixedArray<float, 2>& position,
        float font_height_pixels,
        float line_distance_pixels,
        const std::list<Focus>& focus,
        const std::string& text);
    ~LoadingTextLogic();

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    
    virtual Focus focus_mask() const override;

private:
    const std::list<Focus>& focus_;
    const std::string text_;
};

}
