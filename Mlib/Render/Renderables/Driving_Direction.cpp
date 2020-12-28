#include "Driving_Direction.hpp"

using namespace Mlib;

DrivingDirection Mlib::driving_direction_from_string(const std::string& s) {
    if (s == "center") {
        return DrivingDirection::CENTER;
    } else if (s == "left") {
        return DrivingDirection::LEFT;
    } else if (s == "right") {
        return DrivingDirection::RIGHT;
    } else {
        throw std::runtime_error("Unknown driving direction: " + s);
    }
}
