#include "Screen_Units.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

ScreenUnits Mlib::screen_units_from_string(const std::string& str) {
    static const std::unordered_map<std::string, ScreenUnits> m{
        {"pixels", ScreenUnits::PIXELS},
        {"pixels_fp", ScreenUnits::PIXELS_FP},
        {"inches", ScreenUnits::INCHES}
    };
    auto it = m.find(str);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown screen units: \"" + str + '"');
    }
    return it->second;
}

float Mlib::to_pixels(ScreenUnits units, float value, float dpi) {
    switch (units) {
    case ScreenUnits::PIXELS:
        return std::round(value);
    case ScreenUnits::PIXELS_FP:
        return value;
    case ScreenUnits::INCHES:
        return dpi * value;
    default:
        THROW_OR_ABORT("Unknown screen units");
    }
}

FixedArray<float, 2> Mlib::to_pixels(
    ScreenUnits units,
    const FixedArray<float, 2>& value,
    const FixedArray<float, 2>& dpi)
{
    switch (units) {
    case ScreenUnits::PIXELS:
        return round(value);
    case ScreenUnits::PIXELS_FP:
        return value;
    case ScreenUnits::INCHES:
        return dpi * value;
    default:
        THROW_OR_ABORT("Unknown screen units");
    }
}
