#pragma once
#include <cstddef>

namespace Mlib {

class RigidBodyPulses;
template <class TDir, class TPos, size_t tsize>
struct VectorAtPosition;

void jump(
    RigidBodyPulses& o0,
    RigidBodyPulses& o1,
    float dv,
    const VectorAtPosition<float, double, 3>& vp);

}
