#include "Fill_Pixel_Region_With_Texture_Logic.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>

using namespace Mlib;

FillPixelRegionWithTextureLogic::FillPixelRegionWithTextureLogic(
    const std::string& image_resource_name,
    const FixedArray<float, 2>& position,
    const FixedArray<float, 2>& size,
    ResourceUpdateCycle update_cycle,
    FocusFilter focus_filter)
: FillWithTextureLogic{image_resource_name, update_cycle},
  position_{position},
  size_{size},
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
    auto vg = ViewportGuard::periodic(
        position_(0),
        position_(1),
        size_(0),
        size_(1),
        width,
        height);
    if (vg.has_value()) {
        FillWithTextureLogic::render(width, height, render_config, scene_graph_config, render_results, frame_id);
    }
}

FocusFilter FillPixelRegionWithTextureLogic::focus_filter() const {
    return focus_filter_;
}
