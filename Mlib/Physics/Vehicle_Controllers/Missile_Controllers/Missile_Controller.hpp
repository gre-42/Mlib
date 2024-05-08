#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Missile_Controllers/Rigid_Body_Missile_Controller.hpp>
#include <cstddef>
#include <map>

namespace Mlib {

struct MissileWingController {
    size_t i;
    FixedArray<float, 2> gain;
    float antiroll_angle;
};

class MissileController: public RigidBodyMissileController {
public:
    MissileController(
        RigidBodyVehicle& rb,
        std::vector<MissileWingController> wing_controllers);
    virtual ~MissileController() override;
    virtual void apply() override;
private:
    std::vector<MissileWingController> wing_controllers_;
};

}
