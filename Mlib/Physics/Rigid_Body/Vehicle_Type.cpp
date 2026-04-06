
#include "Vehicle_Type.hpp"
#include <map>
#include <stdexcept>

using namespace Mlib;

VehicleType Mlib::vehicle_type_from_string(const std::string& s) {
    static const std::map<std::string, VehicleType> m{
        { "avatar", VehicleType::AVATAR },
        { "car", VehicleType::CAR },
        { "helicopter", VehicleType::HELICOPTER },
        { "missile", VehicleType::MISSILE},
        { "plane", VehicleType::PLANE },
        { "ship", VehicleType::SHIP },
        { "skateboard", VehicleType::SKATEBOARD }};
    auto it = m.find(s);
    if (it == m.end()) {
        throw std::runtime_error("Unknown vehicle type: \"" + s + '"');
    }
    return it->second;
}
