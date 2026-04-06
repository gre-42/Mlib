#include "Visual_Movable_Text_Logger.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Layout/ILayout_Pixels.hpp>
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Macro_Executor/Expression_Watcher.hpp>
#include <Mlib/Misc/Log.hpp>
#include <Mlib/OpenGL/Render_Setup.hpp>
#include <Mlib/OpenGL/Text/Align_Text.hpp>
#include <Mlib/OpenGL/Text/Charsets.hpp>
#include <Mlib/OpenGL/Text/Renderable_Text.hpp>
#include <Mlib/OpenGL/Text/Text_Interpolation_Mode.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <sstream>

using namespace Mlib;

VisualMovableTextLogger::VisualMovableTextLogger(
    const DanglingBaseClassRef<StatusWriter>& status_writer,
    StatusComponents log_components,
    std::unique_ptr<ExpressionWatcher>&& ew,
    std::string charset,
    std::string ttf_filename,
    std::unique_ptr<IWidget>&& widget,
    const ILayoutPixels& font_height,
    const ILayoutPixels& line_distance)
    : RenderTextLogic{
        ascii,
        std::move(ttf_filename),
        {1.f, 1.f, 1.f},
        font_height,
        line_distance }
    , ew_{ std::move(ew) }
    , charset_{ std::move(charset) }
    , status_writer_{ status_writer }
    , log_components_{ log_components }
    , widget_{ std::move(widget) }
{
    if (!contains_all(status_writer_->status_component(), log_components_)) {
        throw std::runtime_error("Status component not supported: " + std::to_string((int)log_components_));
    }
}

VisualMovableTextLogger::~VisualMovableTextLogger() = default;

void VisualMovableTextLogger::advance_time(float dt, const StaticWorld& world) {
    std::stringstream sstr;
    status_writer_->write_status(sstr, log_components_, world);
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
        font_height_.to_pixels(ly, PixelsRoundMode::ROUND),
        *widget_->evaluate(lx, ly, YOrientation::AS_IS, RegionRoundMode::ENABLED),
        (std::string)text_,
        line_distance_.to_pixels(ly, PixelsRoundMode::NONE),
        TextInterpolationMode::NEAREST_NEIGHBOR,
        GenericTextAlignment::DEFAULT,
        GenericTextAlignment::DEFAULT);
}
