
#include "Clear_Mode.hpp"
#include <stdexcept>

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
        throw std::runtime_error("Unknown clear mode");
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
    throw std::runtime_error("Unknown clear mode: \"" + str + '"');
}
