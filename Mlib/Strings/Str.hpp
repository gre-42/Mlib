#pragma once
#include <Mlib/Strings/Encoding.hpp>
#include <filesystem>
#include <string>

namespace Mlib {
namespace U8 {

inline std::string str(const Mlib::u8string& s) {
    return std::string(s.begin(), s.end());
}

inline std::string_view str(const Mlib::u8string_view& s) {
    return std::string_view((const char*)s.data(), s.size());
}

inline const char* str(const u8char* s) {
    return (const char*)s;
}

inline Mlib::u8string u8str(const std::string& s) {
    return Mlib::u8string(s.begin(), s.end());
}

inline Mlib::u8string_view u8str(const std::string_view& s) {
    return Mlib::u8string_view((const u8char*)s.data(), s.size());
}

inline const u8char* u8str(const char* s) {
    return (const u8char*)s;
}

}
}
