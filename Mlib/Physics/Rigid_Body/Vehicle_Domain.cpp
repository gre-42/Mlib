#include "Vehicle_Domain.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace Mlib;

VehicleDomain Mlib::vehicle_domain_from_string(const std::string& str) {
    static const std::map<std::string, VehicleDomain> m{
        {"undefined", VehicleDomain::UNDEFINED},
        {"air", VehicleDomain::AIR},
        {"ground", VehicleDomain::GROUND}
    };
    auto it = m.find(str);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown vehicle domain: \"" + str + '"');
    }
    return it->second;
}

std::string Mlib::vehicle_domain_to_string(VehicleDomain domain) {
    switch (domain) {
    case VehicleDomain::UNDEFINED:
        return "undefined";
    case VehicleDomain::AIR:
        return "air";
    case VehicleDomain::GROUND:
        return "ground";
    case VehicleDomain::END:
        ; // Fall through
    }
    THROW_OR_ABORT("Unknown vehicle domain: " + std::to_string((int)domain));
}
