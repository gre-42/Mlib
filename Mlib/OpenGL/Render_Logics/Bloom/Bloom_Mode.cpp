
#include "Bloom_Mode.hpp"
#include <map>
#include <stdexcept>

using namespace Mlib;

BloomMode Mlib::bloom_mode_from_string(const std::string& s) {
    const std::map<std::string, BloomMode> m{
        {"standard", BloomMode::STANDARD},
        {"sky", BloomMode::SKY}
    };
    auto it = m.find(s);
    if (it == m.end()) {
        throw std::runtime_error("Unknown bloom mode: \"" + s + '"');
    }
    return it->second;
}
