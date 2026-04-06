#include "Driving_Direction.hpp"
#include <map>
#include <stdexcept>

using namespace Mlib;

DrivingDirection Mlib::driving_direction_from_string(const std::string& s) {
    static const std::map<std::string, DrivingDirection> m{
        {"center", DrivingDirection::CENTER},
        {"left", DrivingDirection::LEFT},
        {"right", DrivingDirection::RIGHT}};
    auto it = m.find(s);
    if (it == m.end()) {
        throw std::runtime_error("Unknown driving direction: \"" + s + '"');
    }
    return it->second;
}

std::string Mlib::driving_direction_to_string(DrivingDirection direction) {
    switch (direction) {
    case DrivingDirection::CENTER:
        return "center";
    case DrivingDirection::LEFT:
        return "left";
    case DrivingDirection::RIGHT:
        return "right";
    }
    throw std::runtime_error("Unknown driving direction: " + std::to_string((uint32_t)direction));
}
