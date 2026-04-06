#pragma once
#include <Mlib/Hashing/Variable_And_Hash.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <iosfwd>
#include <string>
#include <vector>

namespace Mlib {

struct StaticWorld;

enum class StatusComponents {
    NONE = 0,
    TIME = 1 << 0,
    POSITION = 1 << 1,
    SPEED = 1 << 2,
    HEALTH = 1 << 3,
    DIAMETER = 1 << 4,
    ENERGY = 1 << 5,
    DRIVER_NAME = 1 << 6,
    ANGULAR_VELOCITY = 1 << 7,
    WHEEL_ANGULAR_VELOCITY = 1 << 8,
    ABS_ANGULAR_VELOCITY = 1 << 9
};

inline bool any(StatusComponents a) {
    return a != StatusComponents::NONE;
}

inline StatusComponents operator & (StatusComponents a, StatusComponents b) {
    return (StatusComponents)((int)a & (int)b);
}

inline StatusComponents operator | (StatusComponents a, StatusComponents b) {
    return (StatusComponents)((int)a | (int)b);
}

inline StatusComponents& operator |= (StatusComponents& a, StatusComponents b) {
    (int&)a |= (int)b;
    return a;
}

inline bool contains_all(StatusComponents a, StatusComponents b) {
    return (a & b) == b;
}

class StatusWriter: public virtual DanglingBaseClass {
public:
    virtual StatusComponents status_component() const = 0;
    virtual void write_status(
        std::ostream& ostr,
        StatusComponents status_components,
        const StaticWorld& world) const = 0;
    virtual float get_value(StatusComponents status_components) const = 0;
    virtual DanglingBaseClassRef<StatusWriter> child_status_writer(const std::vector<VariableAndHash<std::string>>& name) = 0;
};

StatusComponents status_components_from_string(const std::string& s);

}
