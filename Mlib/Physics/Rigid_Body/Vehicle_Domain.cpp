#include "Vehicle_Domain.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

VehicleDomain Mlib::vehicle_domain_from_string(const std::string& str) {
    const std::map<std::string, VehicleDomain> m{
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
