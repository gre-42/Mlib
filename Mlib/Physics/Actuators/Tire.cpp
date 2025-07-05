#include "Tire.hpp"
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;

Tire::Tire(
    const VariableAndHash<std::string>& engine,
    std::optional<VariableAndHash<std::string>> delta_engine,
    RigidBodyPulses* rbp,
    float brake_force,
    float brake_torque,
    float sKs,
    float sKa,
    float sKe,
    const Interp<float>& stiction_coefficient,
    const CombinedMagicFormula<float>& magic_formula,
    const FixedArray<float, 3>& vehicle_mount_0,
    const FixedArray<float, 3>& vehicle_mount_1,
    float radius)
    : BaseRotor{ engine, std::move(delta_engine), rbp, brake_torque }
    , magic_formula{ magic_formula }
    , shock_absorber_position{ 0 }
    , angle_x{ 0 }
    , angle_y{ 0 }
    // accel_x{0}
    , brake_force{ brake_force }
    , sKs{ sKs }
    , sKa{ sKa }
    , sKe{ sKe }
    , stiction_coefficient{ stiction_coefficient }
    , vehicle_mount_0{ vehicle_mount_0 }
    , vehicle_mount_1{ vehicle_mount_1 }
    , vertical_line{ uninitialized }
    , radius{ radius }
    , normal_impulse{ nullptr }
{
    vertical_line = (vehicle_mount_1 - vehicle_mount_0);
    auto len2 = sum(squared(vertical_line));
    if (len2 < 1e-12) {
        THROW_OR_ABORT("Tire vehicle mount points are identical");
    }
    vertical_line /= std::sqrt(len2);
}

Tire::~Tire() = default;

void Tire::advance_time(float dt) {
    angle_x = std::fmod(angle_x + dt * angular_velocity, float(2 * M_PI));
    normal_impulse = nullptr;
}

FixedArray<float, 3> Tire::rotation_axis() const {
    return FixedArray<float, 3>{ std::cos(angle_y), 0.f, -std::sin(angle_y) };
}
