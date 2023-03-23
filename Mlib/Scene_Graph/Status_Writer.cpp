#include "Status_Writer.hpp"
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

void StatusWriter::write_status(std::ostream& ostr, StatusComponents status_components) const {
    THROW_OR_ABORT("StatusWriter::write_status not implemented");
}

float StatusWriter::get_value(StatusComponents status_components) const {
    THROW_OR_ABORT("StatusWriter::get_value not implemented");
}

StatusComponents single_status_component_from_string(const std::string& s) {
    if (s == "time") {
        return StatusComponents::TIME;
    }
    if (s == "position") {
        return StatusComponents::POSITION;
    }
    if (s == "speed") {
        return StatusComponents::SPEED;
    }
    if (s == "health") {
        return StatusComponents::HEALTH;
    }
    if (s == "acceleration") {
        return StatusComponents::ACCELERATION;
    }
    if (s == "diameter") {
        return StatusComponents::DIAMETER;
    }
    if (s == "diameter2") {
        return StatusComponents::DIAMETER2;
    }
    if (s == "energy") {
        return StatusComponents::ENERGY;
    }
    if (s == "driver_name") {
        return StatusComponents::DRIVER_NAME;
    }
    if (s == "angular_velocity") {
        return StatusComponents::ANGULAR_VELOCITY;
    }
    if (s == "wheel_angular_velocity") {
        return StatusComponents::WHEEL_ANGULAR_VELOCITY;
    }
    if (s == "abs_angular_velocity") {
        return StatusComponents::ABS_ANGULAR_VELOCITY;
    }
    throw std::runtime_error("Unknown status component: \"" + s + '"');
}

StatusComponents Mlib::status_components_from_string(const std::string& s) {
    static const DECLARE_REGEX(re, "\\|");
    StatusComponents result = StatusComponents::NONE;
    for (const auto& m : string_to_list(s, re)) {
        result |= single_status_component_from_string(m);
    }
    return result;
}
