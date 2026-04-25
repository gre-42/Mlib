#include "Encoding.hpp"
#include <Mlib/Misc/Pragma_Gcc.hpp>
#include <Mlib/Strings/Str.hpp>
#ifndef __EMSCRIPTEN__
PRAGMA_GCC_DIAGNOSTIC_PUSH
PRAGMA_GCC_DIAGNOSTIC_IGNORED(-Wsign-conversion)
#include <boost/regex/pending/unicode_iterator.hpp>
PRAGMA_GCC_DIAGNOSTIC_POP
#endif

using namespace Mlib;

#ifndef __EMSCRIPTEN__
Generator<char32_t> Mlib::u8_to_u32_generator(const std::string& utf8) {
    boost::u8_to_u32_iterator it(utf8.begin());
    boost::u8_to_u32_iterator tend(utf8.end());
    for (; it != tend; ++it) {
        co_yield *it;
    }
}

Generator<char> Mlib::u32_to_u8_generator(const std::u32string& utf32) {
    boost::u32_to_u8_iterator it(utf32.begin());
    boost::u32_to_u8_iterator tend(utf32.end());
    for (; it != tend; ++it) {
        co_yield (char)*it;
    }
}
#else
Generator<char32_t> Mlib::u8_to_u32_generator(const std::string& utf8) {
    for (auto c : utf8) {
        if (!is_ascii(c)) {
            throw std::runtime_error("Detected non-ASCII character");
        }
        co_yield (char32_t)c;
    }
}

Generator<char> Mlib::u32_to_u8_generator(const std::u32string& utf32) {
    for (auto c : utf32) {
        if (!is_ascii32(c)) {
            throw std::runtime_error("Detected non-ASCII character");
        }
        co_yield (char)c;
    }
}
#endif

std::u32string Mlib::u8_to_u32_string(const std::string& utf8) {
    // std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> wconv;
    // return wconv.from_bytes(utf8);
    std::u32string result;
    result.reserve(nchars32(utf8));
    for (auto c : u8_to_u32_generator(utf8)) {
        result += c;
    }
    return result;
}

std::string Mlib::u32_to_u8_string(const std::u32string& wstr) {
    // std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> wconv;
    // return wconv.to_bytes(wstr);
    size_t n = 0;
    for ([[maybe_unused]] auto c : u32_to_u8_generator(wstr)) {
        ++n;
    }
    std::string result;
    result.reserve(n);
    for (auto c : u32_to_u8_generator(wstr)) {
        result += c;
    }
    return result;
}

size_t Mlib::nchars32(const std::string& utf8) {
    size_t n = 0;
    for ([[maybe_unused]] auto c : u8_to_u32_generator(utf8)) {
        ++n;
    }
    return n;
}

std::u32string Mlib::ascii_to_u32_string(const std::string& ascii) {
    return { ascii.begin(), ascii.end() };
}

std::string Mlib::u32_to_ascii_string(const std::u32string& wstr) {
    std::string result(wstr.length(), '?');
    for (size_t i = 0; i < wstr.length(); ++i) {
        unsigned char c = (unsigned char)wstr[i];
        if (c == wstr[i]) {
            result[i] = (char)c;
        }
    }
    return result;
}
