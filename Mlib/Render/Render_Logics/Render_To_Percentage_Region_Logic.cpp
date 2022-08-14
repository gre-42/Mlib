#include "Render_To_Percentage_Region_Logic.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>

using namespace Mlib;

RenderToPercentageRegionLogic::RenderToPercentageRegionLogic(
    RenderLogic& render_logic,
    const FixedArray<float, 2>& position,
    const FixedArray<float, 2>& size,
    const FocusFilter& focus_filter,
    bool flip_y)
: render_logic_{render_logic},
  position_{position},
  size_{size},
  focus_filter_{focus_filter},
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
    FixedArray<int, 2> iposition{
        (int)std::round(position_(0) * width),
        (int)std::round(position_(1) * height)};
    FixedArray<int, 2> isize{
        (int)std::round(size_(0) * width),
        (int)std::round(size_(1) * height)};
    ViewportGuard vg{
        iposition(0),
        flip_y_ ? height - iposition(1) - isize(1) : iposition(1),
        isize(0),
        isize(1)};
    render_logic_.render(
        isize(0),
        isize(1),
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
