#include "Interpolation_Mode.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace std::string_view_literals;
using namespace Mlib;

InterpolationMode Mlib::interpolation_mode_from_string(std::string_view str) {
    if (str == "nearest"sv) {
        return InterpolationMode::NEAREST;
    } else if (str == "linear"sv) {
        return InterpolationMode::LINEAR;
    } else {
        THROW_OR_ABORT("Unknown interpolation mode: \"" + std::string{str} + '"');
    }
}
