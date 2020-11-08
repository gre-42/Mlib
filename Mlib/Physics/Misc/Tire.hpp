#pragma once
#include <Mlib/Physics/Misc/Shock_Absorber.hpp>
#include <Mlib/Physics/Misc/Sticky_Wheel.hpp>
#include <Mlib/Physics/Misc/Tracking_Wheel.hpp>
#include <string>

namespace Mlib {

struct Tire {
    Tire(
        const std::string& engine,
        float break_force,
        const ShockAbsorber& shock_absorber,
        const StickyWheel& sticky_wheel,
        const TrackingWheel& tracking_wheel,
        float angle,
        const FixedArray<float, 3>& position);
    void advance_time(float dt);
    ShockAbsorber shock_absorber;
    StickyWheel sticky_wheel;
    TrackingWheel tracking_wheel;
    float angle;
    std::string engine;
    float break_force;
    FixedArray<float, 3> position;
};

}
