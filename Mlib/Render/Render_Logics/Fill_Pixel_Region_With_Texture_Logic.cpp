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
: fill_with_texture_logic_{image_resource_name, update_cycle},
  widget_{std::move(widget)},
  focus_filter_{std::move(focus_filter)}
{}

void FillPixelRegionWithTextureLogic::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("FillPixelRegionWithTextureLogic::render");
    auto vg = ViewportGuard::from_widget(*widget_->evaluate(lx, ly, YOrientation::AS_IS));
    if (vg.has_value()) {
        fill_with_texture_logic_.render();
    }
}

FocusFilter FillPixelRegionWithTextureLogic::focus_filter() const {
    return focus_filter_;
}

void FillPixelRegionWithTextureLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "FillPixelRegionWithTextureLogic\n";
}
