#pragma once
#include <Mlib/Geometry/Vector_At_Position.hpp>
#include <string>

namespace Mlib {

struct Rotor {
    Rotor(
        const std::string& engine,
        const VectorAtPosition<float, 3>& location);
    std::string engine;
    VectorAtPosition<float, 3> location;
    float angular_velocity;
};

}
