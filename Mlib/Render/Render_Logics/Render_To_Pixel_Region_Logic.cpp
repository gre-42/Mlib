#include "Render_To_Pixel_Region_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>

using namespace Mlib;

RenderToPixelRegionLogic::RenderToPixelRegionLogic(
    RenderLogic& render_logic,
    const FixedArray<float, 2>& position,
    const FixedArray<float, 2>& size,
    const FocusFilter& focus_filter)
: render_logic_{render_logic},
  position_{position},
  size_{size},
  focus_filter_{focus_filter}
{}

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
    auto vg = ViewportGuard::periodic(
        position_(0),
        position_(1),
        size_(0),
        size_(1),
        width,
        height);
    if (vg.has_value()) {
        render_logic_.render(
            size_(0),
            size_(1),
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
