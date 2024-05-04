#include "Way_Point_Location.hpp"
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace Mlib;

static const std::map<std::string, WayPointLocation> m{
    { "unknown", WayPointLocation::UNKNOWN },
    { "street", WayPointLocation::STREET },
    { "sidewalk", WayPointLocation::SIDEWALK },
    { "explicit_ground", WayPointLocation::EXPLICIT_GROUND }
};

std::string Mlib::way_point_location_to_string(WayPointLocation t) {
    auto cmp = WayPointLocation::NONE;
    std::string result;
    for (const auto& [name, value] : m) {
        if (any(t & value)) {
            if (result.empty()) {
                result = name;
            } else {
                result += '|' + name;
            }
            cmp |= value;
        }
    }
    if (cmp != t) {
        THROW_OR_ABORT("Unknown waypoint location");
    }
    return result;
}

WayPointLocation Mlib::way_point_location_from_string(const std::string& s) {
    auto result = WayPointLocation::NONE;
    static const DECLARE_REGEX(re, "\\|");
    for (const auto& t : string_to_list(s, re)) {
        auto it = m.find(t);
        if (it == m.end()) {
            THROW_OR_ABORT("Unknown waypoint location: \"" + t + '"');
        }
        result |= it->second;
    }
    return result;
}
