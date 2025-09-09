#include "Render_To_Pixel_Region_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <Mlib/Scene_Graph/Rendered_Scene_Descriptor.hpp>
#include <ostream>
#include <string>

using namespace Mlib;

RenderToPixelRegionLogic::RenderToPixelRegionLogic(
    RenderLogic& render_logic,
    std::unique_ptr<IWidget>&& widget,
    FocusFilter focus_filter)
    : on_render_logic_destroy{ render_logic.on_destroy, CURRENT_SOURCE_LOCATION }
    , render_logic_{ render_logic }
    , widget_{ std::move(widget) }
    , focus_filter_{ std::move(focus_filter) }
{}

RenderToPixelRegionLogic::~RenderToPixelRegionLogic() {
    on_destroy.clear();
}

std::optional<RenderSetup> RenderToPixelRegionLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return render_logic_.try_render_setup(lx, ly, frame_id);
}

bool RenderToPixelRegionLogic::render_optional_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id,
    const RenderSetup* setup)
{
    LOG_FUNCTION("RenderToPixelRegionLogic::render");
    if ((frame_id.external_render_pass.pass != ExternalRenderPassType::STANDARD) &&
        !any(frame_id.external_render_pass.pass & ExternalRenderPassType::FOREGROUND_MASK))
    {
        return true;
    }
    auto ew = widget_->evaluate(lx, ly, YOrientation::AS_IS, RegionRoundMode::ENABLED);
    auto vg = ViewportGuard::from_widget(*ew);
    if (vg.has_value()) {
        render_logic_.render_auto_setup(
            LayoutConstraintParameters::child_x(lx, *ew),
            LayoutConstraintParameters::child_y(ly, *ew),
            render_config,
            scene_graph_config,
            render_results,
            frame_id,
            setup);
    }
    return true;
}

bool RenderToPixelRegionLogic::is_visible(const UiFocus& ui_focus) const {
    std::shared_lock lock{ ui_focus.focuses.mutex };
    return ui_focus.has_focus(focus_filter_);
}

void RenderToPixelRegionLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "RenderToPixelRegionLogic\n";
    render_logic_.print(ostr, depth + 1);
}
