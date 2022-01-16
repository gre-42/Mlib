#include "Vehicle_Domain.hpp"
#include <stdexcept>

using namespace Mlib;

VehicleDomain Mlib::vehicle_domain_from_string(const std::string& str) {
    if (str == "air") {
        return VehicleDomain::AIR;
    } else if (str == "ground") {
        return VehicleDomain::GROUND;
    } else {
        throw std::runtime_error("Unknown vehicle domain: \"" + str + '"');
    }
}
