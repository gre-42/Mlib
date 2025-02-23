#pragma once
#include <generator>
#include <string>

namespace Mlib {

std::generator<char32_t> u8_to_u32_generator(const std::string& utf8);
std::generator<char> u32_to_u8_generator(const std::u32string& utf32);

std::u32string u8_to_u32_string(const std::string& utf8);
std::string u32_to_u8_string(const std::u32string& wstr);

std::u32string ascii_to_u32_string(const std::string& ascii);
std::string u32_to_ascii_string(const std::u32string& wstr);

}
