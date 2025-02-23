#include "Encoding.hpp"
#include <boost/regex/pending/unicode_iterator.hpp>

using namespace Mlib;

#ifdef _MSC_VER
std::experimental::generator<char32_t> Mlib::u8_to_u32_generator(const std::string& utf8) {
#else
std::generator<char32_t> Mlib::u8_to_u32_generator(const std::string& utf8) {
#endif
    boost::u8_to_u32_iterator it(utf8.begin());
    boost::u8_to_u32_iterator tend(utf8.end());
    for (; it != tend; ++it) {
        co_yield *it;
    }
}

#ifdef _MSC_VER
std::experimental::generator<char> Mlib::u32_to_u8_generator(const std::u32string& utf32) {
#else
std::generator<char> Mlib::u32_to_u8_generator(const std::u32string& utf32) {
#endif
    boost::u32_to_u8_iterator it(utf32.begin());
    boost::u32_to_u8_iterator tend(utf32.end());
    for (; it != tend; ++it) {
        co_yield *it;
    }
}

std::u32string Mlib::u8_to_u32_string(const std::string& utf8) {
    // std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> wconv;
    // return wconv.from_bytes(utf8);
    std::u32string result;
    result.reserve(utf8.size());
    for (auto c : u8_to_u32_generator(utf8)) {
        result += c;
    }
    result.shrink_to_fit();
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

std::u32string Mlib::ascii_to_u32_string(const std::string& ascii) {
    return { ascii.begin(), ascii.end() };
}

std::string Mlib::u32_to_ascii_string(const std::u32string& wstr) {
    std::string result(wstr.length(), '?');
    for (size_t i = 0; i < wstr.length(); ++i) {
        unsigned char c = (unsigned char)wstr[i];
        if (c == wstr[i]) {
            result[i] = c;
        }
    }
    return result;
}
