#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Render_Logics/Fill_Pixel_Region_With_Texture_Logic.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>

namespace Mlib {

enum class ResourceUpdateCycle;

class ControlsLogic: public RenderLogic {
public:
    ControlsLogic(
        const std::string& gamepad_texture,
        const FixedArray<float, 2>& position,
        const FixedArray<float, 2>& size,
        const FocusFilter& focus_filter);

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    
    virtual FocusFilter focus_filter() const override;

private:
    FillPixelRegionWithTextureLogic gamepad_texture_;
    FocusFilter focus_filter_;
};

}
