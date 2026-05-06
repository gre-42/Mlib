#include "Joined_Way_Point_Sandbox.hpp"
#include <Mlib/Regex/Split.hpp>
#include <map>
#include <stdexcept>

using namespace Mlib;

static const std::map<std::string, JoinedWayPointSandbox> m{
    { "street", JoinedWayPointSandbox::STREET },
    { "sidewalk", JoinedWayPointSandbox::SIDEWALK },
    { "explicit_ground", JoinedWayPointSandbox::EXPLICIT_GROUND },
    { "runway_or_taxiway_or_airway", JoinedWayPointSandbox::RUNWAY_OR_TAXIWAY_OR_AIRWAY }
};

std::string Mlib::joined_way_point_sandbox_to_string(JoinedWayPointSandbox t) {
    auto cmp = JoinedWayPointSandbox::NONE;
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
        throw std::runtime_error("Unknown joined waypoint sandbox");
    }
    return result;
}

JoinedWayPointSandbox Mlib::joined_way_point_sandbox_from_string(const std::string& s) {
    auto result = JoinedWayPointSandbox::NONE;
    static const DECLARE_REGEX(re, "\\|");
    for (const auto& t : string_to_list(s, re)) {
        auto it = m.find(t);
        if (it == m.end()) {
            throw std::runtime_error("Unknown joined waypoint sandbox: \"" + t + '"');
        }
        result |= it->second;
    }
    return result;
}
