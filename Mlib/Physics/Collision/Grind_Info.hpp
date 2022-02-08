#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

class RigidBodyVehicle;

struct GrindInfo {
    float squared_distance;
    FixedArray<float, 3> intersection_point;
    RigidBodyVehicle* rail_rb;
};

}