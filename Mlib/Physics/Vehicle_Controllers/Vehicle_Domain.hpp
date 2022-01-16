#pragma once
#include <string>

namespace Mlib {

enum class VehicleDomain {
    AIR,
    GROUND
};

VehicleDomain vehicle_domain_from_string(const std::string& str);

}
