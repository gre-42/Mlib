#include "Vertical_Subdivision.hpp"
#include <Mlib/Regex/Split.hpp>
#include <map>
#include <stdexcept>

using namespace std::string_view_literals;
using namespace Mlib;

static VerticalSubdivision single_vertical_subdivision_from_string(const std::string& s) {
    static const std::map<std::string_view, VerticalSubdivision> m{
        {"none"sv, VerticalSubdivision::NONE},
        {"socle"sv, VerticalSubdivision::SOCLE},
        {"entrances"sv, VerticalSubdivision::ENTRANCES}
    };
    auto it = m.find(s);
    if (it == m.end()) {
        throw std::runtime_error("Unknown vertical subdivision: \"" + s + '"');
    }
    return it->second;
}

VerticalSubdivision Mlib::vertical_subdivision_from_string(const std::string& s) {
    static const DECLARE_REGEX(re, "\\|");
    VerticalSubdivision result = VerticalSubdivision::NONE;
    for (const auto& m : string_to_list(s, re)) {
        result |= single_vertical_subdivision_from_string(m);
    }
    return result;
}
