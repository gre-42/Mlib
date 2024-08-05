#pragma once
#include <Mlib/Scene_Pos.hpp>
#include <cstddef>

namespace Mlib {

class RigidBodyPulses;
template <class TDir, class TPos, size_t tsize>
struct VectorAtPosition;

void jump(
    RigidBodyPulses& o0,
    RigidBodyPulses& o1,
    float dv,
    const VectorAtPosition<float, ScenePos, 3>& vp);

}
