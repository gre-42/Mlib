#pragma once
#include <string>

namespace Mlib {

enum class VehicleDomain {
    UNDEFINED,
    AIR,
    GROUND,
    END
};

VehicleDomain vehicle_domain_from_string(const std::string& str);
std::string vehicle_domain_to_string(VehicleDomain domain);

}
