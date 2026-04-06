
#include "Rigid_Body_Vehicle_Flags.hpp"
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Strings/String.hpp>
#include <map>
#include <stdexcept>

using namespace Mlib;

RigidBodyVehicleFlags single_rigid_body_vehicle_flags_from_string(const std::string& s) {
    static const std::map<std::string, RigidBodyVehicleFlags> m{
        {"none", RigidBodyVehicleFlags::NONE},
        {"feels_no_gravity", RigidBodyVehicleFlags::FEELS_NO_GRAVITY},
        {"is_deactivated_avatar", RigidBodyVehicleFlags::IS_DEACTIVATED_AVATAR},
        {"is_activated_avatar", RigidBodyVehicleFlags::IS_ACTIVATED_AVATAR}
    };
    auto it = m.find(s);
    if (it == m.end()) {
        throw std::runtime_error("Unknown vehicle flags: \"" + s + '"');
    }
    return it->second;
}

RigidBodyVehicleFlags Mlib::rigid_body_vehicle_flags_from_string(const std::string& s) {
    static const DECLARE_REGEX(re, "\\|");
    RigidBodyVehicleFlags result = RigidBodyVehicleFlags::NONE;
    for (const auto& m : string_to_list(s, re)) {
        result |= single_rigid_body_vehicle_flags_from_string(m);
    }
    return result;
}
