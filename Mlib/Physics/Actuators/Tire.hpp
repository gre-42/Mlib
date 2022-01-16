#pragma once
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Physics/Collision/Magic_Formula.hpp>
#include <Mlib/Physics/Misc/Shock_Absorber.hpp>
#include <Mlib/Physics/Misc/Tracking_Wheel.hpp>
#include <string>

namespace Mlib {

/**
 * Represents a tire.
 *
 * References: https://en.wikipedia.org/wiki/Tire_load_sensitivity
 */
struct Tire {
    Tire(
        const std::string& engine,
        float break_force,
        float sKs,
        float sKa,
        const Interp<float>& stiction_coefficient,
        const Interp<float>& friction_coefficient,
        const ShockAbsorber& shock_absorber,
        const TrackingWheel& tracking_wheel,
        const CombinedMagicFormula<float>& magic_formula,
        const FixedArray<float, 3>& position,
        float radius);
    void advance_time(float dt);
    ShockAbsorber shock_absorber;
    TrackingWheel tracking_wheel;
    CombinedMagicFormula<float> magic_formula;
    float shock_absorber_position;
    float angle_x;
    float angle_y;
    float angular_velocity;
    // float accel_x;
    std::string engine;
    float break_force;
    float sKs;
    float sKa;
    Interp<float> stiction_coefficient;
    Interp<float> friction_coefficient;
    FixedArray<float, 3> position;
    float radius;
};

}
