#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <cstddef>
#include <map>

namespace Mlib {

enum class VehicleDomain;

class PlaneAsCarController: public RigidBodyVehicleController {
public:
    PlaneAsCarController(
        RigidBodyVehicle& rb,
        const std::map<size_t, float>& tire_angles,
        VehicleDomain vehicle_domain);
    virtual ~PlaneAsCarController() override;
    virtual void apply() override;
private:
    std::map<size_t, float> tire_angles_;
    VehicleDomain vehicle_domain_;
};

}
