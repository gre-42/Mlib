#include "Way_Point_Sandbox.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace Mlib;

std::string Mlib::way_point_sandbox_to_string(WayPointSandbox t) {
    switch (t) {
    case WayPointSandbox::STREET:
        return "street";
    case WayPointSandbox::SIDEWALK:
        return "sidewalk";
    case WayPointSandbox::EXPLICIT_GROUND:
        return "explicit_ground";
    case WayPointSandbox::RUNWAY_OR_TAXIWAY:
        return "runway_or_taxiway";
    }
    THROW_OR_ABORT("Unknown waypoint sandbox");
}

WayPointSandbox Mlib::way_point_sandbox_from_string(const std::string& s) {
    static const std::map<std::string, WayPointSandbox> m{
        { "street", WayPointSandbox::STREET },
        { "sidewalk", WayPointSandbox::SIDEWALK },
        { "explicit_ground", WayPointSandbox::EXPLICIT_GROUND },
        { "runway_or_taxiway", WayPointSandbox::RUNWAY_OR_TAXIWAY }
    };
    auto it = m.find(s);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown waypoint sandbox: \"" + s + '"');
    }
    return it->second;
}
