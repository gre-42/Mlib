#include "Vertical_Subdivision.hpp"
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

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
        THROW_OR_ABORT("Unknown vertical subdivision: \"" + s + '"');
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
