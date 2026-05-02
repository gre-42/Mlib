
#include "Texture_Warn_Flags.hpp"
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Regex/Split.hpp>
#include <map>
#include <stdexcept>

using namespace Mlib;

static TextureWarnFlags single_texture_warn_flag_from_string(const std::string& s) {
    static const std::map<std::string, TextureWarnFlags> m{
        {"power_of_two", TextureWarnFlags::POWER_OF_TWO},
        {"too_many_channels", TextureWarnFlags::TOO_MANY_CHANNELS}
    };
    auto it = m.find(s);
    if (it == m.end()) {
        throw std::runtime_error("Unknown warn flag: \"" + s + '"');
    }
    return it->second;
}

TextureWarnFlags Mlib::texture_warn_flags_from_string(const std::string& s) {
    static const DECLARE_REGEX(re, "\\|");
    TextureWarnFlags result = TextureWarnFlags::NONE;
    for (const auto& m : string_to_list(s, re)) {
        result |= single_texture_warn_flag_from_string(m);
    }
    return result;
}
