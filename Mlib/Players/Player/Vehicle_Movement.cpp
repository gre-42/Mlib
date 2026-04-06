#include "Vehicle_Movement.hpp"
#include <cmath>
#include <stdexcept>

using namespace Mlib;

VehicleMovement::VehicleMovement()
: surface_power_forward_{ NAN },
  surface_power_backward_{ NAN }
{}

VehicleMovement::~VehicleMovement()
{}

void VehicleMovement::set_control_parameters(
    float surface_power_forward,
    float surface_power_backward)
{
    if (!std::isnan(surface_power_forward_)) {
        throw std::runtime_error("surface_power_forward already set");
    }
    surface_power_forward_ = surface_power_forward;
    if (!std::isnan(surface_power_backward_)) {
        throw std::runtime_error("surface_power_backward already set");
    }
    surface_power_backward_ = surface_power_backward;
}

void VehicleMovement::reset_node() {
    surface_power_forward_ = NAN;
    surface_power_backward_ = NAN;
}

float VehicleMovement::surface_power_forward() const {
    return surface_power_forward_;
}

float VehicleMovement::surface_power_backward() const {
    return surface_power_backward_;
}
