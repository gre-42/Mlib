#pragma once
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Physics/Actuators/Base_Rotor.hpp>
#include <Mlib/Physics/Collision/Pacejkas_Magic_Formula.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <string>

namespace Mlib {

class RigidBodyPulses;
struct NormalImpulse;

/**
 * Represents a tire.
 *
 * References: https://en.wikipedia.org/wiki/Tire_load_sensitivity
 */
class Tire: public BaseRotor {
    Tire(const Tire&) = delete;
    Tire& operator = (const Tire&) = delete;
public:
    Tire(
        const VariableAndHash<std::string>& engine,
        std::optional<VariableAndHash<std::string>> delta_engine,
        RigidBodyPulses* rbp,
        float brake_force,
        float brake_torque,
        float sKs,
        float sKa,
        float sKe,
        const Interp<float>& stiction_coefficient,
        const CombinedPacejkasMagicFormula<float>& magic_formula,
        const FixedArray<float, 3>& vehicle_mount_0,
        const FixedArray<float, 3>& vehicle_mount_1,
        float radius);
    ~Tire();
    void advance_time(float dt);
    FixedArray<float, 3> rotation_axis() const;
    CombinedPacejkasMagicFormula<float> magic_formula;
    float shock_absorber_position;
    float angle_x;
    float angle_y;
    // float accel_x;
    float brake_force;
    float sKs;
    float sKa;
    float sKe;
    Interp<float> stiction_coefficient;
    FixedArray<float, 3> vehicle_mount_0;
    FixedArray<float, 3> vehicle_mount_1;
    FixedArray<float, 3> vertical_line;
    float radius;
    const NormalImpulse* normal_impulse;
};

}
