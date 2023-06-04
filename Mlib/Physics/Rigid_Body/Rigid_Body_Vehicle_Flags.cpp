#include "Rigid_Body_Vehicle_Flags.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

RigidBodyVehicleFlags single_rigid_body_vehicle_flags_from_string(const std::string& s) {
    if (s == "none") {
        return RigidBodyVehicleFlags::NONE;
    } else if (s == "feels_no_gravity") {
        return RigidBodyVehicleFlags::FEELS_NO_GRAVITY;
    } else if (s == "is_avatar") {
        return RigidBodyVehicleFlags::IS_AVATAR;
    }
    THROW_OR_ABORT("Unknown vehicle flags: \"" + s + '"');
}

RigidBodyVehicleFlags Mlib::rigid_body_vehicle_flags_from_string(const std::string& s) {
    static const DECLARE_REGEX(re, "\\|");
    RigidBodyVehicleFlags result = RigidBodyVehicleFlags::NONE;
    for (const auto& m : string_to_list(s, re)) {
        result |= single_rigid_body_vehicle_flags_from_string(m);
    }
    return result;
}
