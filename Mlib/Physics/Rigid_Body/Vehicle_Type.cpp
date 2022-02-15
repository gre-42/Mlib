#include "Vehicle_Type.hpp"
#include <stdexcept>

using namespace Mlib;

VehicleType Mlib::vehicle_type_from_string(const std::string& s) {
    if (s == "avatar") {
        return VehicleType::AVATAR;
    } else if (s == "car") {
        return VehicleType::CAR;
    } else if (s == "skateboard") {
        return VehicleType::SKATEBOARD;
    } else {
        throw std::runtime_error("Unknown vehicle type: \"" + s + '"');
    }
}
