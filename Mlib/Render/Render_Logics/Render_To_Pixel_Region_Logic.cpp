#include "Render_To_Pixel_Region_Logic.hpp"
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>

using namespace Mlib;

RenderToPixelRegionLogic::RenderToPixelRegionLogic(
    RenderLogic& render_logic,
    std::unique_ptr<IWidget>&& widget,
    FocusFilter focus_filter)
: render_logic_{render_logic},
  widget_{std::move(widget)},
  focus_filter_{std::move(focus_filter)}
{}

RenderToPixelRegionLogic::~RenderToPixelRegionLogic() = default;

void RenderToPixelRegionLogic::render(
    int width,
    int height,
    float xdpi,
    float ydpi,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("RenderToPixelRegionLogic::render");
    auto vg = ViewportGuard::from_widget(*widget_->evaluate(xdpi, ydpi, width, height, YOrientation::AS_IS));
    if (vg.has_value()) {
        render_logic_.render(
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

FocusFilter RenderToPixelRegionLogic::focus_filter() const {
    return focus_filter_;
}

void RenderToPixelRegionLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "RenderToPixelRegionLogic\n";
    render_logic_.print(ostr, depth + 1);
}
