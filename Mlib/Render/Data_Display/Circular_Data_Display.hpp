#pragma once
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <string>
#include <vector>

namespace Mlib {

struct DisplayTick {
    float value;
    std::string text;
};

class CircularDataDisplay {
public:
    CircularDataDisplay(
        TextResource& tick_text,
        float maximum_value,
        float blank_angle,
        const std::vector<DisplayTick>& ticks);
    void render(
        float value,
        const LayoutConstraintParameters& ly,
        const IPixelRegion& evaluated_widget,
        const ILayoutPixels& tick_radius,
        const ILayoutPixels& inner_value_radius,
        const ILayoutPixels& outer_value_radius);
private:
    void ensure_initialized(
        const LayoutConstraintParameters& ly,
        const FixedArray<float, 2>& canvas_size,
        float tick_radius);
    TextResource& tick_text_;
    float maximum_value_;
    float blank_angle_;
    std::vector<DisplayTick> ticks_;
    bool is_initialized_;
};

}
