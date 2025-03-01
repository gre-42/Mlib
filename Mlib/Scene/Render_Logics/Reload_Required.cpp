#include "Reload_Required.hpp"
#include <Mlib/Layout/ILayout_Pixels.hpp>
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Text/Align_Text.hpp>
#include <Mlib/Render/Text/Charsets.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Text/Text_Interpolation_Mode.hpp>

using namespace Mlib;

ReloadRequired::ReloadRequired(
    std::string title,
    std::string charset,
    std::string ttf_filename,
    std::unique_ptr<IWidget>&& widget,
    const FixedArray<float, 3>& font_color,
    const ILayoutPixels& font_height,
    const ILayoutPixels& line_distance,
    FocusFilter focus_filter,
    MacroLineExecutor mle,
    UiFocus& ui_focus)
    : RenderTextLogic{
        ascii,
        std::move(ttf_filename),
        font_color,
        font_height,
        line_distance }
    , title_{ std::move(title) }
    , charset_{ std::move(charset) }
    , mle_{ std::move(mle) }
    , globals_changed_{ false }
    , ui_focus_{ ui_focus }
    , widget_{ std::move(widget) }
    , focus_filter_{ std::move(focus_filter) }
{
    mle_.add_observer([this](){
        globals_changed_ = true;
    });
}

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
    if (globals_changed_) {
        cached_title_ = mle_.eval<std::string>(title_);
        renderable_text().set_charset(VariableAndHash{mle_.eval<std::string>(charset_)});
    }
    std::stringstream sstr;
    sstr << cached_title_ << '\n';
    for (const auto& [_, v] : r) {
        sstr << "  - " << v << '\n';
    }
    renderable_text().render(
        font_height_.to_pixels(ly, PixelsRoundMode::ROUND),
        *widget_->evaluate(lx, ly, YOrientation::AS_IS, RegionRoundMode::ENABLED),
        sstr.str(),
        line_distance_.to_pixels(ly, PixelsRoundMode::NONE),
        TextInterpolationMode::NEAREST_NEIGHBOR,
        GenericTextAlignment::DEFAULT,
        GenericTextAlignment::DEFAULT);
    globals_changed_ = false;
}

FocusFilter ReloadRequired::focus_filter() const {
    return focus_filter_;
}

void ReloadRequired::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "ReloadRequired\n";
}
