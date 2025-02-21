#include "Reload_Required.hpp"
#include <Mlib/Layout/ILayout_Pixels.hpp>
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Text/Text_Interpolation_Mode.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

using namespace Mlib;

ReloadRequired::ReloadRequired(
    std::string title,
    VariableAndHash<std::string> charset,
    std::string ttf_filename,
    std::unique_ptr<IWidget>&& widget,
    const FixedArray<float, 3>& font_color,
    const ILayoutPixels& font_height,
    const ILayoutPixels& line_distance,
    FocusFilter focus_filter,
    UiFocus& ui_focus)
    : RenderTextLogic{
        std::move(charset),
        std::move(ttf_filename),
        font_color,
        font_height,
        line_distance }
    , title_{ std::move(title) }
    , ui_focus_{ ui_focus }
    , widget_{ std::move(widget) }
    , focus_filter_{ std::move(focus_filter) }
{}

ReloadRequired::~ReloadRequired() {
    on_destroy.clear();
}

std::optional<RenderSetup> ReloadRequired::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return std::nullopt;
}

void ReloadRequired::render_without_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("ReloadRequired::render");
    const auto& r = ui_focus_.requires_reload();
    if (r.empty()) {
        return;
    }
    std::stringstream sstr;
    sstr << title_ << '\n';
    for (const auto& [_, v] : r) {
        sstr << "  - " << v << '\n';
    }
    renderable_text().render(
        font_height_.to_pixels(ly, PixelsRoundMode::ROUND),
        *widget_->evaluate(lx, ly, YOrientation::AS_IS, RegionRoundMode::ENABLED),
        sstr.str(),
        line_distance_.to_pixels(ly, PixelsRoundMode::NONE),
        TextInterpolationMode::NEAREST_NEIGHBOR);
}

FocusFilter ReloadRequired::focus_filter() const {
    return focus_filter_;
}

void ReloadRequired::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "ReloadRequired\n";
}
