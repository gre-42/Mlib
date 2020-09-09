#include "Shock_Absorber.hpp"

using namespace Mlib;

ShockAbsorber::ShockAbsorber(float Ks, float Ka)
: Ks_{Ks},
  Ka_{Ka},
  position_{0},
  F_{0}
{}

void ShockAbsorber::integrate_force(float F) {
    F_ += F;
}

void ShockAbsorber::advance_time(float dt, bool implicit) {
    // Solve: K_spring * x + K_absorber * dx / dt = F
    // Solve: K_spring * x{0,1} + K_absorber * (x1 - x0) / dt = F

    // Implicit: solve(s * x1 + a * (x1 - x0) / t = F, x1);
    // Explicit: solve(s * x0 + a * dx / t = F, dx);

    if (implicit) {
        position_ = (F_ + Ka_ / dt * position_) / (Ks_ + Ka_ / dt);
    } else {
        position_ += dt / Ka_ * (F_ - Ks_ * position_);
    }
    F_ = 0;
}
