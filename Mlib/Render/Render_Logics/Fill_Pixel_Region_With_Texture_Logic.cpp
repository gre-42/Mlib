#include "Fill_Pixel_Region_With_Texture_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Render_Logics/Screen_Units.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>

using namespace Mlib;

FillPixelRegionWithTextureLogic::FillPixelRegionWithTextureLogic(
    const std::string& image_resource_name,
    const FixedArray<float, 2>& position,
    const FixedArray<float, 2>& size,
    ScreenUnits screen_units,
    ResourceUpdateCycle update_cycle,
    FocusFilter focus_filter)
: FillWithTextureLogic{image_resource_name, update_cycle},
  position_{position},
  size_{size},
  screen_units_{screen_units},
  focus_filter_{std::move(focus_filter)}
{}

void FillPixelRegionWithTextureLogic::render(
    int width,
    int height,
    float xdpi,
    float ydpi,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("FillPixelRegionWithTextureLogic::render");
    auto position_pixels = to_pixels(screen_units_, position_, {xdpi, ydpi}, {width, height});
    auto size_pixels = to_pixels(screen_units_, size_, {xdpi, ydpi}, {width, height});
    auto vg = ViewportGuard::periodic(
        position_pixels(0),
        position_pixels(1),
        size_pixels(0),
        size_pixels(1),
        width,
        height);
    if (vg.has_value()) {
        FillWithTextureLogic::render(
            vg.value().iwidth(),
            vg.value().iheight(),
            xdpi,
            ydpi,
            render_config,
            scene_graph_config,
            render_results,
            frame_id);
    }
}

FocusFilter FillPixelRegionWithTextureLogic::focus_filter() const {
    return focus_filter_;
}
