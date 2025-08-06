#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Missile_Controllers/Rigid_Body_Missile_Controller.hpp>
#include <Mlib/Signal/Pid_Controller.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <cstddef>
#include <map>

namespace Mlib {

struct MissileWingController {
    size_t i;
    FixedArray<float, 2> gain = uninitialized;
    float antiroll_angle;
    float max_angle;
};

class MissileController: public RigidBodyMissileController {
public:
    MissileController(
        RigidBodyVehicle& rb,
        float dt_ref,
        const PidController<FixedArray<float, 2>, float>& pid,
        std::vector<MissileWingController> wing_controllers,
        VariableAndHash<std::string> engine_name);
    virtual ~MissileController() override;
    virtual void apply(float dt) override;
private:
    float dt_ref_;
    PidController<FixedArray<float, 2>, float> pid_;
    std::vector<MissileWingController> wing_controllers_;
    VariableAndHash<std::string> engine_name_;
};

}
