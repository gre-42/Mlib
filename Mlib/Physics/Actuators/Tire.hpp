#pragma once
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Physics/Actuators/Base_Rotor.hpp>
#include <Mlib/Physics/Collision/Magic_Formula.hpp>
#include <string>

namespace Mlib {

/**
 * Represents a tire.
 *
 * References: https://en.wikipedia.org/wiki/Tire_load_sensitivity
 */
struct Tire: public BaseRotor {
    Tire(
        const std::string& engine,
        float break_force,
        float sKs,
        float sKa,
        const Interp<float>& stiction_coefficient,
        const Interp<float>& friction_coefficient,
        const CombinedMagicFormula<float>& magic_formula,
        const FixedArray<float, 3>& position,
        float radius);
    void advance_time(float dt);
    CombinedMagicFormula<float> magic_formula;
    float shock_absorber_position;
    float angle_x;
    float angle_y;
    // float accel_x;
    float break_force;
    float sKs;
    float sKa;
    Interp<float> stiction_coefficient;
    Interp<float> friction_coefficient;
    FixedArray<float, 3> position;
    float radius;
};

}
