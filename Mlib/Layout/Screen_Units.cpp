#include "Screen_Units.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

ScreenUnits Mlib::screen_units_from_string(const std::string& str) {
    if (str == "pixels") {
        return ScreenUnits::PIXELS;
    }
    if (str == "inches") {
        return ScreenUnits::INCHES;
    }
    THROW_OR_ABORT("Unknown screen units: \"" + str + '"');
}

float Mlib::to_pixels(ScreenUnits units, float value, float dpi) {
    if (units == ScreenUnits::PIXELS) {
        return value;
    }
    if (units == ScreenUnits::INCHES) {
        return dpi * value;
    }
    THROW_OR_ABORT("Unknown screen units");
}

FixedArray<float, 2> Mlib::to_pixels(
    ScreenUnits units,
    const FixedArray<float, 2>& value,
    const FixedArray<float, 2>& dpi)
{
    if (units == ScreenUnits::PIXELS) {
        return value;
    }
    if (units == ScreenUnits::INCHES) {
        return dpi * value;
    }
    THROW_OR_ABORT("Unknown screen units");
}
