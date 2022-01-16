#include "Shock_Absorber.hpp"
#include <stdexcept>

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

void ShockAbsorber::advance_time(float dt, IntegrationMode integration_mode) {
    // Solve: K_spring * x + K_absorber * dx / dt = F
    // Solve: K_spring * x{0,1} + K_absorber * (x1 - x0) / dt = F

    // Implicit: solve(s * x1 + a * (x1 - x0) / t = F, x1);
    // Explicit: solve(s * x0 + a * dx / t = F, dx);
    // Semiimpl: solve(s * (x1 + x0) / 2 + a * (x1 - x0) / t = F, x1);

    switch(integration_mode) {
        case IntegrationMode::IMPLICIT:
            position_ = (F_ + Ka_ / dt * position_) / (Ks_ + Ka_ / dt);
            break;
        case IntegrationMode::EXPLICIT:
            position_ += dt / Ka_ * (F_ - Ks_ * position_);
            break;
        case IntegrationMode::SEMI_IMPLICIT:
            position_ = -((Ks_ * dt - 2 * Ka_) * position_ - 2 * F_ * dt) / (Ks_ * dt + 2 * Ka_);
            break;
        default:
            throw std::runtime_error("Unknown integration mode");
    }
    F_ = 0;
}
