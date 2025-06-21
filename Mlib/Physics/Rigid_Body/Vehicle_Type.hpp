#pragma once
#include <string>

namespace Mlib {

enum class VehicleType {
    UNDEFINED,
    AVATAR,
    CAR,
    HELICOPTER,
    MISSILE,
    PLANE,
    SHIP,
    SKATEBOARD
};

VehicleType vehicle_type_from_string(const std::string& s);

}
