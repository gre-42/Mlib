#include "Blend_Mode.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <stdexcept>

using namespace Mlib;

BlendMode Mlib::blend_mode_from_string(const std::string& str) {
    if (str == "off") {
        return BlendMode::OFF;
    } else if (str == "invisible") {
        return BlendMode::INVISIBLE;
    } else if (str == "binary_05") {
        return BlendMode::BINARY_05;
    } else if (str == "binary_08") {
        return BlendMode::BINARY_08;
    } else if (str == "semi_continuous_02") {
        return BlendMode::SEMI_CONTINUOUS_02;
    } else if (str == "semi_continuous_08") {
        return BlendMode::SEMI_CONTINUOUS_08;
    } else if (str == "continuous") {
        return BlendMode::CONTINUOUS;
    } else if (str == "binary_05_add") {
        return BlendMode::BINARY_05_ADD;
    }
    THROW_OR_ABORT("Unknown blend mode: \"" + str + '"');
}
