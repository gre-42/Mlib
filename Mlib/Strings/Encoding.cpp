#include "Encoding.hpp"
#include <codecvt>
#include <locale>

using namespace Mlib;

std::wstring Mlib::utf8_to_wstring(const std::string& utf8) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> wconv;
    return wconv.from_bytes(utf8);
}

std::string Mlib::wstring_to_utf8(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> wconv;
    return wconv.to_bytes(wstr);
}

std::wstring Mlib::ascii_to_wstring(const std::string& ascii) {
    return { ascii.begin(), ascii.end() };
}

std::string Mlib::wstring_to_ascii(const std::wstring& wstr) {
    std::string result(wstr.length(), '?');
    for (size_t i = 0; i < wstr.length(); ++i) {
        char c = (char)wstr[i];
        if (c == wstr[i]) {
            result[i] = c;
        }
    }
    return result;
}
