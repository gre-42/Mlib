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
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("FillPixelRegionWithTextureLogic::render");
    if (screen_units_ == ScreenUnits::PIXELS) {
        auto vg = ViewportGuard::periodic(
            position_(0),
            position_(1),
            size_(0),
            size_(1),
            width,
            height);
        if (vg.has_value()) {
            FillWithTextureLogic::render(
                vg.value().iwidth(),
                vg.value().iheight(),
                render_config,
                scene_graph_config,
                render_results,
                frame_id);
        }
    } else if (screen_units_ == ScreenUnits::FRACTION) {
        FixedArray<float, 2> pix_position{
            position_(0) * (float)width,
            position_(1) * (float)height};
        FixedArray<float, 2> pix_size{
            size_(0) * (float)width,
            size_(1) * (float)height};
        auto vg = ViewportGuard::periodic(
            pix_position(0),
            pix_position(1),
            pix_size(0),
            pix_size(1),
            width,
            height);
        if (vg.has_value()) {
            FillWithTextureLogic::render(
                vg.value().iwidth(),
                vg.value().iheight(),
                render_config,
                scene_graph_config,
                render_results,
                frame_id);
        }
    } else {
        THROW_OR_ABORT("Unknown screen units");
    }
}

FocusFilter FillPixelRegionWithTextureLogic::focus_filter() const {
    return focus_filter_;
}
