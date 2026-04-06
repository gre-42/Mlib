#include "Pathes.hpp"
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Strings/String.hpp>
#include <stdexcept>

using namespace Mlib;

std::vector<std::filesystem::path>
    Mlib::split_semicolon_separated_pathes(const std::string& s)
{
    static const DECLARE_REGEX(re, ";");
    return string_to_vector(s, re, [](const auto& s){ return std::filesystem::path{s}; });
}

std::vector<std::pair<std::filesystem::path, std::filesystem::path>>
    Mlib::split_semicolon_separated_pairs_of_pathes(const std::string& s)
{
    static const DECLARE_REGEX(re, ";");
    return string_to_vector(s, re, [](const auto& s)
    {
        static const DECLARE_REGEX(re, "=");
        auto v = string_to_vector(s, re, [](const auto& s){ return std::filesystem::path{s}; });
        if (v.size() != 2) {
            throw std::runtime_error("Could not parse as key=value: " + std::string{s});
        }
        return std::make_pair(v[0], v[1]);
    });
}
