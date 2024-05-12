#include "Wrap_Mode.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace Mlib;

WrapMode Mlib::wrap_mode_from_string(const std::string& str) {
    static const std::map<std::string, WrapMode> m{
        {"repeat", WrapMode::REPEAT},
        {"clamp_to_edge", WrapMode::CLAMP_TO_EDGE},
        {"clamp_to_border", WrapMode::CLAMP_TO_BORDER}
    };
    auto it = m.find(str);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown wrap mode: \"" + str + '"');
    }
    return it->second;
}
