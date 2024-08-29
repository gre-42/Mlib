#include "Status_Writer.hpp"
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace Mlib;

void StatusWriter::write_status(std::ostream& ostr, StatusComponents status_components, const StaticWorld& world) const {
    THROW_OR_ABORT("StatusWriter::write_status not implemented");
}

float StatusWriter::get_value(StatusComponents status_components) const {
    THROW_OR_ABORT("StatusWriter::get_value not implemented");
}

StatusComponents single_status_component_from_string(const std::string& s) {
    static const std::map<std::string, StatusComponents> map{
        {"time", StatusComponents::TIME},
        {"position", StatusComponents::POSITION},
        {"speed", StatusComponents::SPEED},
        {"health", StatusComponents::HEALTH},
        {"diameter", StatusComponents::DIAMETER},
        {"energy", StatusComponents::ENERGY},
        {"driver_name", StatusComponents::DRIVER_NAME},
        {"angular_velocity", StatusComponents::ANGULAR_VELOCITY},
        {"wheel_angular_velocity", StatusComponents::WHEEL_ANGULAR_VELOCITY},
        {"abs_angular_velocity", StatusComponents::ABS_ANGULAR_VELOCITY}
    };
    auto it = map.find(s);
    if (it == map.end()) {
        throw std::runtime_error("Unknown status component: \"" + s + '"');
    }
    return it->second;
}

StatusComponents Mlib::status_components_from_string(const std::string& s) {
    static const DECLARE_REGEX(re, "\\|");
    StatusComponents result = StatusComponents::NONE;
    for (const auto& m : string_to_list(s, re)) {
        result |= single_status_component_from_string(m);
    }
    return result;
}
