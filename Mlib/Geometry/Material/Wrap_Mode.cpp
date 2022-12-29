#include "Wrap_Mode.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

WrapMode Mlib::wrap_mode_from_string(const std::string& str) {
    if (str == "repeat") {
        return WrapMode::REPEAT;
    } else if (str == "clamp_to_edge") {
        return WrapMode::CLAMP_TO_EDGE;
    } else if (str == "clamp_to_border") {
        return WrapMode::CLAMP_TO_BORDER;
    }
    THROW_OR_ABORT("Unknown wrap mode: " + str);
}
