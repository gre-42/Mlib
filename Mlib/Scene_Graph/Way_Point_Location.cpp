#include "Way_Point_Location.hpp"
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

static const std::map<std::string, WayPointLocation> m{
    { "unknown", WayPointLocation::UNKNOWN },
    { "street", WayPointLocation::STREET },
    { "sidewalk", WayPointLocation::SIDEWALK },
    { "runway_or_taxiway", WayPointLocation::RUNWAY_OR_TAXIWAY },
    { "explicit_ground", WayPointLocation::EXPLICIT_GROUND },
    { "runway_or_taxiway_or_airway", WayPointLocation::RUNWAY_OR_TAXIWAY_OR_AIRWAY }
};

std::string Mlib::way_point_location_to_string(WayPointLocation t) {
    WayPointLocation cmp = WayPointLocation::NONE;
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
    WayPointLocation result = WayPointLocation::NONE;
    static const DECLARE_REGEX(re, "\\|");
    for (const auto& t : string_to_list(s, re)) {
        auto it = m.find(t);
        if (it == m.end()) {
            THROW_OR_ABORT("Unknown waypoint location: \"" + s + '"');
        }
        result |= it->second;
    }
    return result;
}
