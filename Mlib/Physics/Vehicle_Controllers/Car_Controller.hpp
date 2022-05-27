#pragma once
#include <Mlib/Physics/Vehicle_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <Mlib/Signal/Pid_Controller.hpp>
#include <cstddef>
#include <map>

namespace Mlib {

class CarController: public RigidBodyVehicleController {
public:
    CarController(
        RigidBodyVehicle* rb,
        const std::map<size_t, float>& tire_max_angles,
        const PidController<float, float>& tire_angle_pid);
    virtual ~CarController() override;
    virtual void apply() override;
private:
    std::map<size_t, float> tire_max_angles_;
    PidController<float, float> tire_angle_pid_;
};

}
