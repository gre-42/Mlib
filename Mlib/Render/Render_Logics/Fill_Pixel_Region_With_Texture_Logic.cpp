#include "Fill_Pixel_Region_With_Texture_Logic.hpp"
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>

using namespace Mlib;

FillPixelRegionWithTextureLogic::FillPixelRegionWithTextureLogic(
    const std::string& image_resource_name,
    std::unique_ptr<IWidget>&& widget,
    ResourceUpdateCycle update_cycle,
    FocusFilter focus_filter)
: FillWithTextureLogic{image_resource_name, update_cycle},
  widget_{std::move(widget)},
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
    ViewportGuard vg{*widget_->evaluate(xdpi, ydpi, width, height, YOrientation::AS_IS)};
    FillWithTextureLogic::render(
        vg.iwidth(),
        vg.iheight(),
        xdpi,
        ydpi,
        render_config,
        scene_graph_config,
        render_results,
        frame_id);
}

FocusFilter FillPixelRegionWithTextureLogic::focus_filter() const {
    return focus_filter_;
}
