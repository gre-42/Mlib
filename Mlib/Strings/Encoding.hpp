#pragma once
#include <Mlib/Iterator/Generator.hpp>
#include <string>
#include <string_view>

namespace Mlib {

inline bool is_ascii(char c) {
    return (unsigned char)c < 128;
}

inline bool is_ascii32(char32_t c) {
    return c < 128;
}

Generator<char32_t> u8_to_u32_generator(const std::string& utf8);
Generator<char> u32_to_u8_generator(const std::u32string& utf32);

size_t nchars32(const std::string& utf8);

std::u32string u8_to_u32_string(const std::string& utf8);
std::string u32_to_u8_string(const std::u32string& wstr);

std::u32string ascii_to_u32_string(const std::string& ascii);
std::string u32_to_ascii_string(const std::u32string& wstr);

#ifdef WITHOUT_ICU
using u8string = std::string;
using u8string_view = std::string_view;
using u8char = char;
#else
using u8string = std::u8string;
using u8string_view = std::u8string_view;
using u8char = char8_t;
#endif

}
