#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>

namespace Mlib {

class RigidBodyVehicle;

struct GrindInfo {
    float squared_distance;
    FixedArray<ScenePos, 3> intersection_point;
    FixedArray<SceneDir, 3> rail_direction;
    RigidBodyVehicle* rail_rb;
};

}