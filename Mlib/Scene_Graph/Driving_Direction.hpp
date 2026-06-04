#pragma once
#include <cstdint>
#include <stdexcept>
#include <string>

namespace Mlib {

enum class DrivingDirection: uint32_t {
    CENTER = 0,
    LEFT = 1,
    RIGHT = 2
};
static const size_t DRIVING_DIRECTION_BITS = 2;

DrivingDirection driving_direction_from_string(const std::string& s);
std::string driving_direction_to_string(DrivingDirection direction);

}
