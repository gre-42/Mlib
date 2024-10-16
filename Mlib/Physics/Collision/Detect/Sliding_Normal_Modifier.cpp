#include "Sliding_Normal_Modifier.hpp"
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Pulses.hpp>

using namespace Mlib;

SlidingNormalModifier::SlidingNormalModifier(
    const RigidBodyPulses& rbp,
    float fac,
    float max_overlap)
    : rbp_{ rbp }
    , fac_{ fac }
    , max_overlap_{ max_overlap }
{}

SlidingNormalModifier::~SlidingNormalModifier() = default;

void SlidingNormalModifier::modify_collision_normal(
    const FixedArray<ScenePos, 3>& position,
    FixedArray<float, 3>& normal,
    float& overlap) const
{
    // auto v = rbp_.velocity_at_position(position);
    auto v = rbp_.v_;
    auto lv = std::sqrt(sum(squared(v)));
    if (lv < 1e-12) {
        return;
    }
    v /= lv;
    normal -= fac_ * v * dot0d(normal, v);
    normal /= std::sqrt(sum(squared(normal)));
    overlap = std::min(overlap, max_overlap_);
}
