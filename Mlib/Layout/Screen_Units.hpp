#pragma once
#include <cstddef>
#include <string>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

enum class ScreenUnits {
    PIXELS,
    INCHES
};

enum class PixelsRoundMode{
    NONE,
    CEIL,
    ROUND,
    FLOOR
};

ScreenUnits screen_units_from_string(const std::string& str);

float to_pixels(ScreenUnits units, float value, float dpi);

FixedArray<float, 2> to_pixels(
    ScreenUnits units,
    const FixedArray<float, 2>& value,
    const FixedArray<float, 2>& dpi);

float round(float value, PixelsRoundMode pixels_round_mode);

}
