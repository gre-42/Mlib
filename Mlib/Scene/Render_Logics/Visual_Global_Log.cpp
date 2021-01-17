#include "Visual_Global_Log.hpp"
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Scene_Graph/Fifo_Log.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <sstream>

using namespace Mlib;

VisualGlobalLog::VisualGlobalLog(
    BaseLog& base_log,
    const std::string& ttf_filename,
    const FixedArray<float, 2>& position,
    float font_height_pixels,
    float line_distance_pixels,
    size_t nentries,
    LogEntrySeverity severity)
: RenderTextLogic{
    ttf_filename,
    position,
    font_height_pixels,
    line_distance_pixels},
  base_log_{base_log},
  nentries_{nentries},
  severity_{severity}
{}

VisualGlobalLog::~VisualGlobalLog()
{}

void VisualGlobalLog::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    std::stringstream sstr;
    base_log_.get_messages(sstr, nentries_, severity_);
    renderable_text().render(position_, sstr.str(), width, height, line_distance_pixels_, true);  // true=periodic_position
}
