#pragma once
#include <Mlib/Iterator/Generator.hpp>
#include <string>

namespace Mlib {

Generator<char32_t> u8_to_u32_generator(const std::string& utf8);
Generator<char> u32_to_u8_generator(const std::u32string& utf32);

size_t nchars32(const std::string& utf8);

std::u32string u8_to_u32_string(const std::string& utf8);
std::string u32_to_u8_string(const std::u32string& wstr);

std::u32string ascii_to_u32_string(const std::string& ascii);
std::string u32_to_ascii_string(const std::u32string& wstr);

}
