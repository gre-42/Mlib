#pragma once
#include <cstddef>
#include <string>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

enum class ScreenUnits {
    PIXELS,
    FRACTION,
    INCHES
};

ScreenUnits screen_units_from_string(const std::string& str);

float to_pixels(ScreenUnits units, float value, float dpi, int screen_npixels);

FixedArray<float, 2> to_pixels(
    ScreenUnits units,
    const FixedArray<float, 2>& value,
    const FixedArray<float, 2>& dpi,
    const FixedArray<int, 2>& screen_npixels);

}
