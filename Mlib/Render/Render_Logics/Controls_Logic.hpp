#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Render_Logics/Fill_Pixel_Region_With_Texture_Logic.hpp>

namespace Mlib {

enum class ResourceUpdateCycle;
struct UiFocus;

class ControlsLogic: public RenderLogic {
public:
    ControlsLogic(
        const std::string& gamepad_texture,
        const FixedArray<float, 2>& position,
        const FixedArray<float, 2>& size,
        UiFocus& ui_focus,
        size_t submenu_id);

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    
    virtual Focus focus_mask() const override;

private:
    FillPixelRegionWithTextureLogic gamepad_texture_;
    UiFocus& ui_focus_;
    size_t submenu_id_;
};

}
