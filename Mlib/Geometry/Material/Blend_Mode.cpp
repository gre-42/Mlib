#include "Blend_Mode.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace Mlib;

BlendMode Mlib::blend_mode_from_string(const std::string& str) {
    static const std::map<std::string, BlendMode> m{
        {"off", BlendMode::OFF},
        {"invisible", BlendMode::INVISIBLE},
        {"binary_05", BlendMode::BINARY_05},
        {"binary_08", BlendMode::BINARY_08},
        {"semi_continuous_02", BlendMode::SEMI_CONTINUOUS_02},
        {"semi_continuous_08", BlendMode::SEMI_CONTINUOUS_08},
        {"continuous", BlendMode::CONTINUOUS},
        {"binary_05_add", BlendMode::BINARY_05_ADD},
        {"continuous_add", BlendMode::CONTINUOUS_ADD}
    };
    auto it = m.find(str);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown blend mode: \"" + str + '"');
    }
    return it->second;
}
