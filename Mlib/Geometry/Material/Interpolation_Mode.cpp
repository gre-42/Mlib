#include "Interpolation_Mode.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

InterpolationMode Mlib::interpolation_mode_from_string(const std::string& str) {
    if (str == "nearest") {
        return InterpolationMode::NEAREST;
    } else if (str == "linear") {
        return InterpolationMode::LINEAR;
    } else {
        THROW_OR_ABORT("Unknown interpolation mode: \"" + str + '"');
    }
}
