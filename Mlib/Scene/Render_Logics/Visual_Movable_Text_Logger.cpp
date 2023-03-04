#include "Visual_Movable_Text_Logger.hpp"
#include <Mlib/Layout/ILayout_Pixels.hpp>
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <sstream>

using namespace Mlib;

VisualMovableTextLogger::VisualMovableTextLogger(
    StatusWriter& status_writer,
    StatusComponents log_components,
    const std::string& ttf_filename,
    std::unique_ptr<IWidget>&& widget,
    const ILayoutPixels& font_height,
    const ILayoutPixels& line_distance)
: RenderTextLogic{
    ttf_filename,
    font_height,
    line_distance},
  status_writer_{status_writer},
  log_components_{log_components},
  widget_{std::move(widget)}
{}

VisualMovableTextLogger::~VisualMovableTextLogger() = default;

void VisualMovableTextLogger::advance_time(float dt) {
    std::stringstream sstr;
    status_writer_.write_status(sstr, log_components_);
    text_ = sstr.str();
}

void VisualMovableTextLogger::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("VisualMovableTextLogger::render");
    renderable_text().render(
        font_height_.to_pixels(ly),
        *widget_->evaluate(lx, ly, YOrientation::AS_IS),
        (std::string)text_,
        line_distance_.to_pixels(ly));
}
