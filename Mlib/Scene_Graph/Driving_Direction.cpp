#include "Driving_Direction.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

DrivingDirection Mlib::driving_direction_from_string(const std::string& s) {
    if (s == "center") {
        return DrivingDirection::CENTER;
    } else if (s == "left") {
        return DrivingDirection::LEFT;
    } else if (s == "right") {
        return DrivingDirection::RIGHT;
    } else {
        THROW_OR_ABORT("Unknown driving direction: " + s);
    }
}
