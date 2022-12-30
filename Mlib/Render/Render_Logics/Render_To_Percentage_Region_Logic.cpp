#include "Render_To_Percentage_Region_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>

using namespace Mlib;

RenderToPercentageRegionLogic::RenderToPercentageRegionLogic(
    RenderLogic& render_logic,
    const FixedArray<float, 2>& position,
    const FixedArray<float, 2>& size,
    FocusFilter focus_filter,
    bool flip_y)
: render_logic_{render_logic},
  position_{position},
  size_{size},
  focus_filter_{std::move(focus_filter)},
  flip_y_{flip_y}
{}

void RenderToPercentageRegionLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("RenderToPercentageRegionLogic::render");
    FixedArray<float, 2> pix_position{
        position_(0) * (float)width,
        position_(1) * (float)height};
    FixedArray<float, 2> pix_size{
        size_(0) * (float)width,
        size_(1) * (float)height};
    ViewportGuard vg{
        pix_position(0),
        flip_y_ ? (float)height - pix_position(1) - pix_size(1) : pix_position(1),
        pix_size(0),
        pix_size(1)};
    render_logic_.render(
        (int)pix_size(0),
        (int)pix_size(1),
        render_config,
        scene_graph_config,
        render_results,
        frame_id);
}

FocusFilter RenderToPercentageRegionLogic::focus_filter() const {
    return focus_filter_;
}

void RenderToPercentageRegionLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "RenderToPercentageRegionLogic\n";
    render_logic_.print(ostr, depth + 1);
}
