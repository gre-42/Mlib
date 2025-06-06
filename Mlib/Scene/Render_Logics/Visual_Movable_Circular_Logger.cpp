#include "Visual_Movable_Circular_Logger.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Layout/ILayout_Pixels.hpp>
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Macro_Executor/Expression_Watcher.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Text/Charsets.hpp>
#include <Mlib/Render/Text/Text_Interpolation_Mode.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <Mlib/Variable_And_Hash.hpp>

using namespace Mlib;

VisualMovableCircularLogger::VisualMovableCircularLogger(
    StatusWriter& status_writer,
    StatusComponents log_components,
    std::unique_ptr<ExpressionWatcher>&& ew,
    std::string charset,
    std::string ttf_filename,
    const ColormapWithModifiers& pointer_texture_name,
    std::unique_ptr<IWidget>&& widget,
    const FixedArray<float, 3>& font_color,
    const ILayoutPixels& font_height,
    const ILayoutPixels& tick_radius,
    const ILayoutPixels& pointer_width,
    const ILayoutPixels& pointer_length,
    float minimum_value,
    float maximum_value,
    float blank_angle,
    const std::vector<DisplayTick>& ticks)
    : ew_{ std::move(ew) }
    , charset_{ std::move(charset) }
    , status_writer_{ status_writer }
    , log_components_{ log_components }
    , tick_text_{ ascii, std::move(ttf_filename), font_color }
    , pointer_image_logic_{ RenderingContextStack::primary_rendering_resources().get_texture_lazy(pointer_texture_name) }
    , display_{ tick_text_, pointer_image_logic_, minimum_value, maximum_value, blank_angle, ticks }
    , font_height_{ font_height }
    , tick_radius_{ tick_radius }
    , pointer_width_{ pointer_width }
    , pointer_length_{ pointer_length }
    , widget_{ std::move(widget) }
    , value_{ NAN }
{}

VisualMovableCircularLogger::~VisualMovableCircularLogger() = default;

void VisualMovableCircularLogger::advance_time(float dt, const StaticWorld& world) {
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
    RenderConfigGuard rcg{ render_config, frame_id.external_render_pass.pass, InternalRenderPass::BLENDED_LATE };
    if (ew_->result_may_have_changed()) {
        tick_text_.set_charset(VariableAndHash{ew_->eval<std::string>(charset_)});
    }
    display_.render(
        value_,
        font_height_.to_pixels(ly, PixelsRoundMode::ROUND),
        TextInterpolationMode::NEAREST_NEIGHBOR,
        *widget_->evaluate(lx, ly, YOrientation::AS_IS, RegionRoundMode::ENABLED),
        tick_radius_.to_pixels(ly, PixelsRoundMode::NONE),
        { pointer_width_.to_pixels(ly, PixelsRoundMode::NONE), pointer_length_.to_pixels(ly, PixelsRoundMode::NONE) });
}
