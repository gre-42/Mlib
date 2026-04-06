#pragma once
#include <string>

namespace Mlib {

enum class TextureWarnFlags {
    NONE = 0,
    POWER_OF_TWO = 1 << 0,
    TOO_MANY_CHANNELS = 1 << 1
};

inline bool any(TextureWarnFlags flags) {
    return flags != TextureWarnFlags::NONE;
}

inline TextureWarnFlags& operator |= (TextureWarnFlags& a, TextureWarnFlags b) {
    (int&)a |= (int)b;
    return a;
}

inline TextureWarnFlags operator | (TextureWarnFlags a, TextureWarnFlags b) {
    return (TextureWarnFlags)((int)a | (int)b);
}

inline TextureWarnFlags operator & (TextureWarnFlags a, TextureWarnFlags b) {
    return (TextureWarnFlags)((int)a & (int)b);
}

TextureWarnFlags texture_warn_flags_from_string(const std::string& s);

}
