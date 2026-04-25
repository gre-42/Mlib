#include "Pathes.hpp"
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Strings/Str.hpp>
#include <Mlib/Strings/String.hpp>
#include <stdexcept>

using namespace Mlib;

std::vector<Utf8Path>
    Mlib::split_semicolon_separated_pathes(const Mlib::u8string& s)
{
    static const auto re = Mlib::make_u32regex(";");
    return string_to_vector(s, re, [](const auto& s){ return Utf8Path{s}; });
}

std::vector<std::pair<Utf8Path, Utf8Path>>
    Mlib::split_semicolon_separated_pairs_of_pathes(const Mlib::u8string& s)
{
    static const auto re = Mlib::make_u32regex(";");
    return string_to_vector(s, re, [](const auto& s)
    {
        static const auto re = Mlib::make_u32regex("=");
        auto v = string_to_vector(s, re, [](const auto& s){ return Utf8Path{s}; });
        if (v.size() != 2) {
            throw std::runtime_error("Could not parse as key=value: " + U8::str(s));
        }
        return std::make_pair(v[0], v[1]);
    });
}
