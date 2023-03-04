#include "Visual_Movable_Circular_Logger.hpp"
#include <Mlib/Layout/ILayout_Pixels.hpp>
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>

using namespace Mlib;

VisualMovableCircularLogger::VisualMovableCircularLogger(
    StatusWriter& status_writer,
    StatusComponents log_components,
    const std::string& ttf_filename,
    const std::string& pointer_texture_name,
    std::unique_ptr<IWidget>&& widget,
    const ILayoutPixels& font_height,
    const ILayoutPixels& tick_radius,
    const ILayoutPixels& pointer_width,
    const ILayoutPixels& pointer_length,
    float minimum_value,
    float maximum_value,
    float blank_angle,
    const std::vector<DisplayTick>& ticks)
: status_writer_{status_writer},
  log_components_{log_components},
  tick_text_{ttf_filename},
  pointer_image_logic_{pointer_texture_name},
  display_{tick_text_, pointer_image_logic_, minimum_value, maximum_value, blank_angle, ticks},
  font_height_{font_height},
  tick_radius_{tick_radius},
  pointer_width_{pointer_width},
  pointer_length_{pointer_length},
  widget_{std::move(widget)},
  value_{NAN}
{}

VisualMovableCircularLogger::~VisualMovableCircularLogger() = default;

void VisualMovableCircularLogger::advance_time(float dt) {
    value_ = status_writer_.get_value(log_components_);
}

void VisualMovableCircularLogger::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("VisualMovableCircularLogger::render");
    RenderConfigGuard rcg{ render_config, frame_id.external_render_pass.pass };
    display_.render(
        value_,
        font_height_.to_pixels(ly),
        *widget_->evaluate(lx, ly, YOrientation::AS_IS),
        tick_radius_.to_pixels(ly),
        {pointer_width_.to_pixels(ly), pointer_length_.to_pixels(ly)});
}
