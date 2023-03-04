#pragma once
#include <string>
#include <vector>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
class TextResource;
class PointerImageLogic;
class IPixelRegion;

struct DisplayTick {
    float value;
    std::string text;
};

class CircularDataDisplay {
public:
    CircularDataDisplay(
        TextResource& tick_text,
        PointerImageLogic& pointer_image_logic,
        float maximum_value,
        float blank_angle,
        const std::vector<DisplayTick>& ticks);
    void render(
        float value,
        float font_height,
        const IPixelRegion& evaluated_widget,
        float tick_radius,
        const FixedArray<float, 2>& pointer_size);
private:
    void ensure_initialized(
        float font_height,
        const FixedArray<float, 2>& canvas_size,
        float tick_radius);
    float indicator_angle(float value) const;
    TextResource& tick_text_;
    PointerImageLogic& pointer_image_logic_;
    float maximum_value_;
    float blank_angle_;
    std::vector<DisplayTick> ticks_;
    bool is_initialized_;
};

}
