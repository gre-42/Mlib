#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <Mlib/Signal/Pid_Controller.hpp>
#include <cstddef>
#include <map>

namespace Mlib {

enum class VehicleDomain;

class HeliController: public RigidBodyVehicleController {
public:
    HeliController(
        RigidBodyVehicle& rb,
        std::map<size_t, float> tire_angles,
        size_t main_rotor_id,
        FixedArray<float, 3> angle_multipliers,
        const PidController<double, double>& height_pid,
        VehicleDomain vehicle_domain);
    virtual ~HeliController() override;
    virtual void apply() override;
private:
    PidController<double, double> height_pid_;
    std::map<size_t, float> tire_angles_;
    size_t main_rotor_id_;
    FixedArray<float, 3> angle_multipliers_;
    VehicleDomain vehicle_domain_;
};

}
