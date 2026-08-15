#include "Limit_Sources.hpp"
#include <Mlib/Regex/Split.hpp>
#include <map>
#include <stdexcept>
#include <string_view>

using namespace std::string_view_literals;
using namespace Mlib;

static LimitSources single_vertical_subdivision_from_string(const std::string& s) {
    static const std::map<std::string_view, LimitSources> m{
        {"none"sv, LimitSources::NONE},
        {"penetration"sv, LimitSources::PENETRATION},
        {"remote"sv, LimitSources::REMOTE}
    };
    auto it = m.find(s);
    if (it == m.end()) {
        throw std::runtime_error("Unknown limit source: \"" + s + '"');
    }
    return it->second;
}

LimitSources Mlib::limit_sources_from_string(const std::string& s) {
    static const DECLARE_REGEX(re, "\\|");
    LimitSources result = LimitSources::NONE;
    for (const auto& m : string_to_list(s, re)) {
        result |= single_vertical_subdivision_from_string(m);
    }
    return result;
}
