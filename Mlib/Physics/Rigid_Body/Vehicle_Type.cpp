#include "Vehicle_Type.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

VehicleType Mlib::vehicle_type_from_string(const std::string& s) {
    if (s == "avatar") {
        return VehicleType::AVATAR;
    } else if (s == "car") {
        return VehicleType::CAR;
    } else if (s == "plane") {
        return VehicleType::PLANE;
    } else if (s == "helicopter") {
        return VehicleType::HELICOPTER;
    } else if (s == "skateboard") {
        return VehicleType::SKATEBOARD;
    } else {
        THROW_OR_ABORT("Unknown vehicle type: \"" + s + '"');
    }
}
