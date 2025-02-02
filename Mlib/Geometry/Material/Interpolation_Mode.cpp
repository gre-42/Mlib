#include "Interpolation_Mode.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace std::string_view_literals;
using namespace Mlib;

InterpolationMode Mlib::interpolation_mode_from_string(std::string_view str) {
    static const std::map<std::string_view, InterpolationMode> m{
        { "nearest"sv,  InterpolationMode::NEAREST},
        { "linear"sv,  InterpolationMode::LINEAR}
    };
    auto it = m.find(str);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown interpolation mode: \"" + std::string{ str } + '"');
    }
    return it->second;
}

std::string_view Mlib::interpolation_mode_to_string(InterpolationMode mode) {
    switch (mode) {
    case InterpolationMode::NEAREST:
        return "nearest"sv;
    case InterpolationMode::LINEAR:
        return "linear"sv;
    }
    THROW_OR_ABORT("Unknown interpolation mode: " + std::to_string((int)mode));
}
