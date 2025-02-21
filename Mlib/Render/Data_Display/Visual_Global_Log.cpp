#include "Visual_Global_Log.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Layout/ILayout_Pixels.hpp>
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Text/Text_Interpolation_Mode.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Fifo_Log.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <sstream>

using namespace Mlib;

VisualGlobalLog::VisualGlobalLog(
    BaseLog& base_log,
    VariableAndHash<std::string> charset,
    std::string ttf_filename,
    std::unique_ptr<IWidget>&& widget,
    const FixedArray<float, 3>& font_color,
    const ILayoutPixels& font_height,
    const ILayoutPixels& line_distance,
    size_t nentries,
    LogEntrySeverity severity)
    : RenderTextLogic{
        std::move(charset),
        std::move(ttf_filename),
        font_color,
        font_height,
        line_distance }
    , base_log_{ base_log }
    , nentries_{ nentries }
    , severity_{ severity }
    , widget_{ std::move(widget) }
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
    renderable_text().render(
        font_height_.to_pixels(ly, PixelsRoundMode::ROUND),
        *widget_->evaluate(lx, ly, YOrientation::AS_IS, RegionRoundMode::ENABLED),
        sstr.str(),
        line_distance_.to_pixels(ly, PixelsRoundMode::NONE),
        TextInterpolationMode::NEAREST_NEIGHBOR);
}

void VisualGlobalLog::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "VisualGlobalLog\n";
}
