#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Rigid_Body_Plane_Controller.hpp>
#include <Mlib/Signal/Pid_Controller.hpp>
#include <cstddef>
#include <map>

namespace Mlib {

enum class VehicleDomain;

class PlaneController: public RigidBodyPlaneController {
public:
    PlaneController(
        RigidBodyVehicle* rb,
        const std::map<size_t, float>& tire_angles,
        float yaw_amount_to_tire_angle,
        size_t turbine_id,
        VehicleDomain vehicle_domain);
    virtual ~PlaneController() override;
    virtual void apply() override;
private:
    size_t turbine_id_;
    std::map<size_t, float> tire_angles_;
    float yaw_amount_to_tire_angle_;
    VehicleDomain vehicle_domain_;
};

}
