#include "Driving_Direction.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace Mlib;

DrivingDirection Mlib::driving_direction_from_string(const std::string& s) {
    static const std::map<std::string, DrivingDirection> m{
        {"center", DrivingDirection::CENTER},
        {"left", DrivingDirection::LEFT},
        {"right", DrivingDirection::RIGHT}};
    auto it = m.find(s);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown driving direction: \"" + s + '"');
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
    THROW_OR_ABORT("Unknown driving direction: " + std::to_string((uint32_t)direction));
}
