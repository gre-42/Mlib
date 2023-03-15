#include "Pacenote_Display.hpp"
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Render/Text/Align_Text.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Text/Text_And_Position.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>

using namespace Mlib;

PacenoteDisplay::PacenoteDisplay(
    RenderLogicGallery& gallery,
    TextResource& text,
    std::vector<std::string> exhibits)
: gallery_{gallery},
  text_{text},
  exhibits_{std::move(exhibits)}
{}

void PacenoteDisplay::render(
    float value,
    float font_height,
    const IPixelRegion& evaluated_widget)
{
    auto vg = ViewportGuard::from_widget(evaluated_widget);
    if (!vg.has_value()) {
        return;
    }
    FixedArray<float, 2> canvas_size{evaluated_widget.width(), evaluated_widget.height()};
    text_.set_contents(
        font_height,
        canvas_size,
        {TextAndPosition{
            .text = std::to_string(value),
            .position = {0.f, 0.f},
            .align = AlignText::TOP,
            .line_distance = 0.f}});
    text_.render();
}
