#pragma once

namespace Mlib {

class VehicleMovement {
    VehicleMovement(const VehicleMovement&) = delete;
    VehicleMovement& operator = (const VehicleMovement&) = delete;
public:
    VehicleMovement();
    ~VehicleMovement();
    void set_control_parameters(
        float surface_power_forward,
        float surface_power_backward);
    void reset_node();
    float surface_power_forward() const;
    float surface_power_backward() const;
private:
    float surface_power_forward_;
    float surface_power_backward_;
};

}
