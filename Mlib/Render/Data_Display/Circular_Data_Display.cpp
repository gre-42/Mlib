#include "Circular_Data_Display.hpp"
#include <Mlib/Layout/ILayout_Pixels.hpp>
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Render/Text/Align_Text.hpp>
#include <Mlib/Render/Text/Text_And_Position.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>

using namespace Mlib;

CircularDataDisplay::CircularDataDisplay(
    TextResource& tick_text,
    float maximum_value,
    float blank_angle,
    const std::vector<DisplayTick>& ticks)
: tick_text_{tick_text},
  maximum_value_{maximum_value},
  blank_angle_{blank_angle},
  ticks_{ticks},
  is_initialized_{false}
{}

void CircularDataDisplay::render(
    float value,
    const LayoutConstraintParameters& ly,
    const IPixelRegion& evaluated_widget,
    const ILayoutPixels& tick_radius,
    const ILayoutPixels& inner_value_radius,
    const ILayoutPixels& outer_value_radius)
{
    ensure_initialized(
        ly,
        {evaluated_widget.width(), evaluated_widget.height()},
        tick_radius.to_pixels(ly));
    
    auto vg = ViewportGuard::from_widget(evaluated_widget);
    if (vg.has_value()) {
        // float inner_pixels = inner_value_radius.to_pixels(ly);
        // float outer_pixels = outer_value_radius.to_pixels(ly);
        tick_text_.render();
    }
}

void CircularDataDisplay::ensure_initialized(
    const LayoutConstraintParameters& ly,
    const FixedArray<float, 2>& canvas_size,
    float tick_radius)
{
    if (is_initialized_) {
        return;
    }
    std::vector<TextAndPosition> contents;
    contents.reserve(ticks_.size());
    for (const auto& tick : ticks_) {
        float angle = indicator_angle(tick.value);
        contents.push_back(TextAndPosition{
            .text = tick.text,
            .position = canvas_size / 2.f + tick_radius * FixedArray<float, 2>{
                std::cos(angle),
                -std::sin(angle)},
            .align = AlignText::TOP,
            .line_distance = 0.f});
    }
    tick_text_.set_contents(ly, canvas_size, contents);
}

float CircularDataDisplay::indicator_angle(float value) const {
    float p2 = 2.f * float{M_PI};
    float raw_angle = p2 * value / maximum_value_;
    return -float{M_PI} / 2.f - (raw_angle * (p2 - blank_angle_) / p2 + blank_angle_ / 2.f);
}
