#include "Visual_Global_Log.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Fifo_Log.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <sstream>

using namespace Mlib;

VisualGlobalLog::VisualGlobalLog(
    BaseLog& base_log,
    const std::string& ttf_filename,
    const FixedArray<float, 2>& position,
    const FixedArray<float, 2>& size,
    float font_height,
    float line_distance,
    ScreenUnits units,
    size_t nentries,
    LogEntrySeverity severity)
: RenderTextLogic{
    ttf_filename,
    position,
    font_height,
    line_distance,
    units},
  base_log_{base_log},
  nentries_{nentries},
  severity_{severity},
  size_{size}
{}

VisualGlobalLog::~VisualGlobalLog() = default;

void VisualGlobalLog::render(
    int width,
    int height,
    float xdpi,
    float ydpi,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("VisualGlobalLog::render");
    std::stringstream sstr;
    base_log_.get_messages(sstr, nentries_, severity_);
    renderable_text().render(
        {width, height},
        {xdpi, ydpi},
        position_,
        size_,
        sstr.str(),
        line_distance_);
}

void VisualGlobalLog::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "VisualGlobalLog\n";
}
