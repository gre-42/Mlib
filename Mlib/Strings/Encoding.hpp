#pragma once
#include <string>

namespace Mlib {

std::wstring utf8_to_wstring(const std::string& utf8);
std::string wstring_to_utf8(const std::wstring& wstr);

std::wstring ascii_to_wstring(const std::string& ascii);
std::string wstring_to_ascii(const std::wstring& wstr);

}
