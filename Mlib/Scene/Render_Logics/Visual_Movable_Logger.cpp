#include "Visual_Movable_Logger.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <sstream>

using namespace Mlib;

VisualMovableLogger::VisualMovableLogger(
    AdvanceTimes& advance_times,
    StatusWriter* status_writer,
    StatusComponents log_components,
    const std::string& ttf_filename,
    const FixedArray<float, 2>& position,
    const FixedArray<float, 2>& size,
    float font_height,
    float line_distance,
    ScreenUnits units)
: RenderTextLogic{
    ttf_filename,
    position,
    font_height,
    line_distance,
    units},
  advance_times_{advance_times},
  status_writer_{status_writer},
  log_components_{log_components},
  size_{size}
{}

VisualMovableLogger::~VisualMovableLogger()
{}

void VisualMovableLogger::notify_destroyed(Object& destroyed_object) {
    advance_times_.schedule_delete_advance_time(*this);
}

void VisualMovableLogger::advance_time(float dt) {
    std::stringstream sstr;
    status_writer_->write_status(sstr, log_components_);
    text_ = sstr.str();
}

void VisualMovableLogger::render(
    int width,
    int height,
    float xdpi,
    float ydpi,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("VisualMovableLogger::render");
    renderable_text().render({width, height}, {xdpi, ydpi}, position_, size_, text_, line_distance_);
}

void VisualMovableLogger::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "VisualMovableLogger\n";
}
