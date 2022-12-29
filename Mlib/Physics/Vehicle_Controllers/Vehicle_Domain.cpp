#include "Vehicle_Domain.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

VehicleDomain Mlib::vehicle_domain_from_string(const std::string& str) {
    if (str == "air") {
        return VehicleDomain::AIR;
    } else if (str == "ground") {
        return VehicleDomain::GROUND;
    } else {
        THROW_OR_ABORT("Unknown vehicle domain: \"" + str + '"');
    }
}
