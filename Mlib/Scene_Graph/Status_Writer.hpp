#pragma once
#include <iosfwd>
#include <string>
#include <vector>

namespace Mlib {

enum class StatusComponents {
    NONE = 0,
    TIME = 1 << 0,
    POSITION = 1 << 1,
    SPEED = 1 << 2,
    HEALTH = 1 << 3,
    ACCELERATION = 1 << 4,
    DIAMETER = 1 << 5,
    DIAMETER2 = 1 << 6,
    ENERGY = 1 << 7,
    DRIVER_NAME = 1 << 8,
    ANGULAR_VELOCITY = 1 << 9,
    WHEEL_ANGULAR_VELOCITY = 1 << 10,
    ABS_ANGULAR_VELOCITY = 1 << 11
};

inline bool operator & (StatusComponents a, StatusComponents b) {
    return ((unsigned int)a & (unsigned int)b) != 0;
}

inline bool operator |= (StatusComponents& a, StatusComponents b) {
    return (unsigned int&)a |= (unsigned int)b;
}

class StatusWriter {
public:
    virtual void write_status(std::ostream& ostr, StatusComponents status_components) const = 0;
    virtual float get_value(StatusComponents status_components) const = 0;
    virtual StatusWriter& child_status_writer(const std::vector<std::string>& name) = 0;
};

StatusComponents status_components_from_string(const std::string& s);

}
