#pragma once
#include <cstdint>
#include <stdexcept>
#include <string>

namespace Mlib {

enum class DrivingDirection: uint32_t {
    CENTER,
    LEFT,
    RIGHT
};

DrivingDirection driving_direction_from_string(const std::string& s);
std::string driving_direction_to_string(DrivingDirection direction);

}
