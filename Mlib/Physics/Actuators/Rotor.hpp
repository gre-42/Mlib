#pragma once
#include <Mlib/Geometry/Vector_At_Position.hpp>
#include <string>

namespace Mlib {

struct Rotor {
    Rotor(
        const std::string& engine,
        const VectorAtPosition<float, 3>& rest_location,
        float power2lift);
    VectorAtPosition<float, 3> rotated_location() const;
    std::string engine;
    VectorAtPosition<float, 3> rest_location;
    float angle_x;
    float angle_z;
    float angular_velocity;
    float power2lift;
};

}
