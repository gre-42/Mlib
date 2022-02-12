#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

class RigidBodyVehicle;

struct GrindInfo {
    float squared_distance;
    FixedArray<float, 3> intersection_point;
    FixedArray<float, 3> rail_direction;
    RigidBodyVehicle* rail_rb;
};

}