#include "Screen_Units.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

ScreenUnits Mlib::screen_units_from_string(const std::string& str) {
    if (str == "pixels") {
        return ScreenUnits::PIXELS;
    }
    if (str == "fraction") {
        return ScreenUnits::FRACTION;
    }
    THROW_OR_ABORT("Unknown screen units: \"" + str + '"');
}
