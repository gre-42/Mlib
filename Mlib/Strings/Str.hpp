#pragma once
#include <filesystem>
#include <string>

namespace Mlib {
namespace U8 {

inline std::string str(const std::u8string& s) {
    return std::string(s.begin(), s.end());
}

inline std::string_view str(const std::u8string_view& s) {
    return std::string_view((const char*)s.data(), s.size());
}

inline const char* str(const char8_t* s) {
    return (const char*)s;
}

inline std::u8string u8str(const std::string& s) {
    return std::u8string(s.begin(), s.end());
}

inline std::u8string_view u8str(const std::string_view& s) {
    return std::u8string_view((const char8_t*)s.data(), s.size());
}

inline const char8_t* str(const char* s) {
    return (const char8_t*)s;
}

}
}
