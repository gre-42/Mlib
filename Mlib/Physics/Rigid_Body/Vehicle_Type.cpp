#include "Vehicle_Type.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace Mlib;

VehicleType Mlib::vehicle_type_from_string(const std::string& s) {
    static const std::map<std::string, VehicleType> m{
        { "avatar", VehicleType::AVATAR },
        { "car", VehicleType::CAR },
        { "helicopter", VehicleType::HELICOPTER },
        { "missile", VehicleType::MISSILE},
        { "plane", VehicleType::PLANE },
        { "skateboard", VehicleType::SKATEBOARD }};
    auto it = m.find(s);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown vehicle type: \"" + s + '"');
    }
    return it->second;
}
