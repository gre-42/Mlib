#include "Circular_Data_Display.hpp"
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Render/Data_Display/Pointer_Image_Logic.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Text/Align_Text.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Text/Text_And_Position.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <algorithm>

using namespace Mlib;

DisplayTick DisplayTick::from_string(const std::string& s) {
    return DisplayTick{
        .value = safe_stof(s),
        .text = s};
}

CircularDataDisplay::CircularDataDisplay(
    TextResource& tick_text,
    PointerImageLogic& pointer_image_logic,
    float minimum_value,
    float maximum_value,
    float blank_angle,
    std::vector<DisplayTick> ticks)
    : tick_text_{ tick_text }
    , pointer_image_logic_{ pointer_image_logic }
    , minimum_value_{ minimum_value }
    , maximum_value_{ maximum_value }
    , blank_angle_{ blank_angle }
    , ticks_{ std::move(ticks) }
    , is_initialized_{ false }
    , deallocation_token_{ render_deallocator.insert([this](){deallocate();}) }
{}

void CircularDataDisplay::render(
    float value,
    float font_height,
    TextInterpolationMode text_interpolation_mode,
    const IPixelRegion& evaluated_widget,
    float tick_radius,
    const FixedArray<float, 2>& pointer_size)
{
    FixedArray<float, 2> canvas_size{evaluated_widget.width(), evaluated_widget.height()};
    ensure_initialized(
        font_height,
        text_interpolation_mode,
        canvas_size,
        tick_radius);
    
    auto vg = ViewportGuard::from_widget(evaluated_widget);
    FixedArray<float, 2> p00{-pointer_size(0), 0.f};
    FixedArray<float, 2> p10{+pointer_size(0), 0.f};
    FixedArray<float, 2> p01{-pointer_size(0), pointer_size(1)};
    FixedArray<float, 2> p11{+pointer_size(0), pointer_size(1)};
    if (vg.has_value()) {
        tick_text_.render();
        if (!std::isnan(value)) {
            pointer_image_logic_.render(
                canvas_size,
                indicator_angle(std::clamp(value, minimum_value_, maximum_value_)),
                canvas_size / 2.f,
                FixedArray<float, 2, 2, 2>::init(
                    p00(0), p01(0),
                    p10(0), p11(0),
                    p00(1), p01(1),
                    p10(1), p11(1)));
        }
    }
}

void CircularDataDisplay::ensure_initialized(
    float font_height,
    TextInterpolationMode text_interpolation_mode,
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
    tick_text_.set_contents(font_height, canvas_size, text_interpolation_mode, contents);
    is_initialized_ = true;
}

float CircularDataDisplay::indicator_angle(float value) const {
    float p2 = 2.f * float(M_PI);
    float raw_angle = p2 * (value - minimum_value_) / (maximum_value_ - minimum_value_);
    return -float(M_PI) / 2.f - (raw_angle * (p2 - blank_angle_) / p2 + blank_angle_ / 2.f);
}

void CircularDataDisplay::deallocate() {
    is_initialized_ = false;
}
