#pragma once
#include <stdexcept>
#include <string>

namespace Mlib {

enum class DrivingDirection {
    CENTER,
    LEFT,
    RIGHT
};

DrivingDirection driving_direction_from_string(const std::string& s);

}
