#pragma once
#include <iosfwd>

namespace Mlib {

enum class StatusComponents {
    TIME = 1 << 0,
    POSITION = 1 << 1,
    SPEED = 1 << 2,
    HEALTH = 1 << 3,
    ACCELERATION = 1 << 4,
    DIAMETER = 1 << 5,
    DIAMETER2 = 1 << 6,
    ENERGY = 1 << 7,
    DRIVER_NAME = 1 << 8,
    ANGULAR_VELOCITY = 1 << 9
};

inline bool operator & (StatusComponents a, StatusComponents b) {
    return ((unsigned int)a & (unsigned int)b) != 0;
}

class StatusWriter {
public:
    virtual void write_status(std::ostream& ostr, StatusComponents status_components) const = 0;
};

}
