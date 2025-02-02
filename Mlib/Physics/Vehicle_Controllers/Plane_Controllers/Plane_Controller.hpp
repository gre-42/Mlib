#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Plane_Controllers/Rigid_Body_Plane_Controller.hpp>
#include <cstddef>
#include <vector>

namespace Mlib {

class PlaneController: public RigidBodyPlaneController {
public:
    PlaneController(
        RigidBodyVehicle& rb,
        std::vector<size_t> left_front_aileron_wing_ids,
        std::vector<size_t> right_front_aileron_wing_ids,
        std::vector<size_t> left_rear_aileron_wing_ids,
        std::vector<size_t> right_rear_aileron_wing_ids,
        std::vector<size_t> left_rudder_wing_ids,
        std::vector<size_t> right_rudder_wing_ids,
        std::vector<size_t> left_flap_wing_ids,
        std::vector<size_t> right_flap_wing_ids);
    virtual ~PlaneController() override;
    virtual void apply() override;
private:
    std::vector<size_t> left_front_aileron_wing_ids_;
    std::vector<size_t> right_front_aileron_wing_ids_;
    std::vector<size_t> left_rear_aileron_wing_ids_;
    std::vector<size_t> right_rear_aileron_wing_ids_;
    std::vector<size_t> left_rudder_wing_ids_;
    std::vector<size_t> right_rudder_wing_ids_;
    std::vector<size_t> left_flap_wing_ids_;
    std::vector<size_t> right_flap_wing_ids_;
};

}
