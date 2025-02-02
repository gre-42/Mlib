#include "Jump.hpp"
#include <Mlib/Geometry/Vector_At_Position.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Pulses.hpp>

using namespace Mlib;

void Mlib::jump(
    RigidBodyPulses& o0,
    RigidBodyPulses& o1,
    float dv,
    const VectorAtPosition<float, ScenePos, 3>& vp)
{
    if (o0.mass_ == INFINITY) {
        float mc = o1.effective_mass(vp);
        float lambda = - mc * dv;
        o1.integrate_impulse({
            .vector = -vp.vector * lambda,
            .position = vp.position});
    } else {
        float mc0 = o0.effective_mass(vp);
        float mc1 = o1.effective_mass(vp);
        float lambda = - (mc0 * mc1 / (mc0 + mc1)) * dv;
        o0.integrate_impulse({
            .vector = vp.vector * lambda,
            .position = vp.position});
        o1.integrate_impulse({
            .vector = -vp.vector * lambda,
            .position = vp.position});
    }
}
