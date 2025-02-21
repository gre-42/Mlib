#include "Encoding.hpp"
#include <codecvt>
#include <locale>

using namespace Mlib;

std::wstring Mlib::utf8_to_wstring(const std::string& utf8) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> wconv;
    return wconv.from_bytes(utf8);
}
