#include "Visual_Global_Log.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Layout/ILayout_Pixels.hpp>
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Macro_Executor/Expression_Watcher.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Text/Align_Text.hpp>
#include <Mlib/Render/Text/Charsets.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Text/Text_Interpolation_Mode.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Fifo_Log.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <sstream>

using namespace Mlib;

VisualGlobalLog::VisualGlobalLog(
    BaseLog& base_log,
    std::unique_ptr<ExpressionWatcher>&& ew,
    std::string charset,
    std::string ttf_filename,
    std::unique_ptr<IWidget>&& widget,
    const FixedArray<float, 3>& font_color,
    const ILayoutPixels& font_height,
    const ILayoutPixels& line_distance,
    size_t nentries,
    LogEntrySeverity severity,
    FocusFilter focus_filter)
    : RenderTextLogic{
        ascii,
        std::move(ttf_filename),
        font_color,
        font_height,
        line_distance }
    , base_log_{ base_log }
    , ew_{ std::move(ew) }
    , charset_{ std::move(charset) }
    , nentries_{ nentries }
    , severity_{ severity }
    , widget_{ std::move(widget) }
    , focus_filter_{ std::move(focus_filter) }
{}

VisualGlobalLog::~VisualGlobalLog() {
    on_destroy.clear();
}

std::optional<RenderSetup> VisualGlobalLog::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return std::nullopt;
}

void VisualGlobalLog::render_without_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("VisualGlobalLog::render");
    std::stringstream sstr;
    base_log_.get_messages(sstr, nentries_, severity_);
    if (ew_->result_may_have_changed()) {
        renderable_text().set_charset(VariableAndHash{ew_->eval<std::string>(charset_)});
    }
    renderable_text().render(
        font_height_.to_pixels(ly, PixelsRoundMode::ROUND),
        *widget_->evaluate(lx, ly, YOrientation::AS_IS, RegionRoundMode::ENABLED),
        sstr.str(),
        line_distance_.to_pixels(ly, PixelsRoundMode::NONE),
        TextInterpolationMode::NEAREST_NEIGHBOR,
        GenericTextAlignment::DEFAULT,
        GenericTextAlignment::DEFAULT);
}

FocusFilter VisualGlobalLog::focus_filter() const {
    return focus_filter_;
}

void VisualGlobalLog::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "VisualGlobalLog\n";
}
