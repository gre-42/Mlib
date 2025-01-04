#pragma once
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <string>
#include <vector>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
class TextResource;
class PointerImageLogic;
class IPixelRegion;
enum class TextInterpolationMode;

struct DisplayTick {
    float value;
    std::string text;
    static DisplayTick from_string(const std::string& s);
};

class CircularDataDisplay {
public:
    CircularDataDisplay(
        TextResource& tick_text,
        PointerImageLogic& pointer_image_logic,
        float minimum_value,
        float maximum_value,
        float blank_angle,
        std::vector<DisplayTick> ticks);
    void render(
        float value,
        float font_height,
        TextInterpolationMode text_interpolation_mode,
        const IPixelRegion& evaluated_widget,
        float tick_radius,
        const FixedArray<float, 2>& pointer_size);
private:
    void ensure_initialized(
        float font_height,
        TextInterpolationMode text_interpolation_mode,
        const FixedArray<float, 2>& canvas_size,
        float tick_radius);
    void deallocate();
    float indicator_angle(float value) const;
    TextResource& tick_text_;
    PointerImageLogic& pointer_image_logic_;
    float minimum_value_;
    float maximum_value_;
    float blank_angle_;
    std::vector<DisplayTick> ticks_;
    bool is_initialized_;
    DeallocationToken deallocation_token_;
};

}
