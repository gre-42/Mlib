#include "Fill_Pixel_Region_With_Texture_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Render/Render_Logics/Clear_Mode.hpp>
#include <Mlib/Render/Render_Logics/Delay_Load_Policy.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <ostream>
#include <string>

using namespace Mlib;

FillPixelRegionWithTextureLogic::FillPixelRegionWithTextureLogic(
    std::shared_ptr<FillWithTextureLogic> fill_with_texture_logic,
    std::unique_ptr<IWidget>&& widget,
    DelayLoadPolicy delay_load_policy,
    FocusFilter focus_filter)
    : fill_with_texture_logic_{ std::move(fill_with_texture_logic) }
    , widget_{ std::move(widget) }
    , delay_load_policy_{ delay_load_policy }
    , focus_filter_{ std::move(focus_filter) }
{}

FillPixelRegionWithTextureLogic::~FillPixelRegionWithTextureLogic() {
    on_destroy.clear();
}

std::optional<RenderSetup> FillPixelRegionWithTextureLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return std::nullopt;
}

void FillPixelRegionWithTextureLogic::render_without_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("FillPixelRegionWithTextureLogic::render");
    if (delay_load_policy_ == DelayLoadPolicy::DELAY) {
        if (!fill_with_texture_logic_->texture_is_loaded_and_try_preload()) {
            return;
        }
    }
    auto vg = ViewportGuard::from_widget(*widget_->evaluate(lx, ly, YOrientation::AS_IS, RegionRoundMode::ENABLED));
    if (vg.has_value()) {
        fill_with_texture_logic_->render(ClearMode::OFF);
    }
}

FocusFilter FillPixelRegionWithTextureLogic::focus_filter() const {
    return focus_filter_;
}

void FillPixelRegionWithTextureLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "FillPixelRegionWithTextureLogic\n";
}
