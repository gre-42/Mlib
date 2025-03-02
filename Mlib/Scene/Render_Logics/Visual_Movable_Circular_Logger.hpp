#pragma once
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Render/Data_Display/Circular_Data_Display.hpp>
#include <Mlib/Render/Data_Display/Pointer_Image_Logic.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Scene/Render_Logics/Visual_Movable_Logger_View.hpp>
#include <atomic>
#include <cstddef>

namespace Mlib {

enum class StatusComponents;
class StatusWriter;
class IWidget;
template <typename TData, size_t... tshape>
class FixedArray;
class ExpressionWatcher;

class VisualMovableCircularLogger: public VisualMovableLoggerView {
public:
    VisualMovableCircularLogger(
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
        const std::vector<DisplayTick>& ticks);
    virtual ~VisualMovableCircularLogger();

    virtual void advance_time(float dt, const StaticWorld& world) override;

    virtual void render(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;

private:
    std::unique_ptr<ExpressionWatcher> ew_;
    std::string charset_;
    StatusWriter& status_writer_;
    StatusComponents log_components_;
    TextResource tick_text_;
    PointerImageLogic pointer_image_logic_;
    CircularDataDisplay display_;
    const ILayoutPixels& font_height_;
    const ILayoutPixels& tick_radius_;
    const ILayoutPixels& pointer_width_;
    const ILayoutPixels& pointer_length_;
    std::unique_ptr<IWidget> widget_;
    std::atomic<float> value_;
};

}
