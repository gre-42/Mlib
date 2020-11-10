#pragma once
#include <Mlib/Physics/Misc/Shock_Absorber.hpp>
#include <Mlib/Physics/Misc/Tracking_Wheel.hpp>
#include <string>

namespace Mlib {

struct Tire {
    Tire(
        const std::string& engine,
        float break_force,
        const ShockAbsorber& shock_absorber,
        const TrackingWheel& tracking_wheel,
        const FixedArray<float, 3>& position,
        float radius);
    void advance_time(float dt);
    ShockAbsorber shock_absorber;
    TrackingWheel tracking_wheel;
    float angle_x;
    float angle_y;
    float angular_velocity;
    std::string engine;
    float break_force;
    FixedArray<float, 3> position;
    float radius;
};

}
