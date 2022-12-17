#include "Clear_Mode.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

std::string Mlib::clear_mode_to_string(ClearMode clear_mode) {
    switch (clear_mode) {
    case ClearMode::OFF:
        return "off";
    case ClearMode::COLOR:
        return "color";
    case ClearMode::DEPTH:
        return "depth";
    case ClearMode::COLOR_AND_DEPTH:
        return "color_and_depth";
    default:
        THROW_OR_ABORT("Unknown clear mode");
    }
}

ClearMode Mlib::clear_mode_from_string(const std::string& str) {
    if (str == "off") {
        return ClearMode::OFF;
    }
    if (str == "color") {
        return ClearMode::COLOR;
    }
    if (str == "depth") {
        return ClearMode::DEPTH;
    }
    if (str == "color_and_depth") {
        return ClearMode::COLOR_AND_DEPTH;
    }
    THROW_OR_ABORT("Unknown clear mode: \"" + str + '"');
}
