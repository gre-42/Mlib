
#include "Wrap_Mode.hpp"
#include <map>
#include <stdexcept>

using namespace Mlib;

WrapMode Mlib::wrap_mode_from_string(const std::string& str) {
    static const std::map<std::string, WrapMode> m{
        {"repeat", WrapMode::REPEAT},
        {"clamp_to_edge", WrapMode::CLAMP_TO_EDGE},
        {"clamp_to_border", WrapMode::CLAMP_TO_BORDER}
    };
    auto it = m.find(str);
    if (it == m.end()) {
        throw std::runtime_error("Unknown wrap mode: \"" + str + '"');
    }
    return it->second;
}

std::string Mlib::wrap_mode_to_string(WrapMode wrap_mode) {
    switch (wrap_mode) {
        case WrapMode::REPEAT: return "repeat";
        case WrapMode::CLAMP_TO_EDGE: return "clamp_to_edge";
        case WrapMode::CLAMP_TO_BORDER: return "clamp_to_border";
    }
    throw std::runtime_error("Unknown wrap mode: " + std::to_string((int)wrap_mode));
}
