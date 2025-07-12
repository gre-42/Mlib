#include "Bloom_Mode.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace Mlib;

BloomMode Mlib::bloom_mode_from_string(const std::string& s) {
    const std::map<std::string, BloomMode> m{
        {"standard", BloomMode::STANDARD},
        {"sky", BloomMode::SKY}
    };
    auto it = m.find(s);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown bloom mode: \"" + s + '"');
    }
    return it->second;
}
