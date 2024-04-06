#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Missile_Controllers/Rigid_Body_Missile_Controller.hpp>
#include <cstddef>
#include <map>

namespace Mlib {

struct MissileWingController {
    size_t i;
    float gain;
};

class MissileController: public RigidBodyMissileController {
public:
    MissileController(
        RigidBodyVehicle& rb,
        const MissileWingController& left_front,
        const MissileWingController& right_front,
        const MissileWingController& down_front,
        const MissileWingController& up_front,
        const MissileWingController& left_rear,
        const MissileWingController& right_rear,
        const MissileWingController& down_rear,
        const MissileWingController& up_rear);
    virtual ~MissileController() override;
    virtual void apply() override;
private:
    MissileWingController left_front_;
    MissileWingController right_front_;
    MissileWingController down_front_;
    MissileWingController up_front_;
    MissileWingController left_rear_;
    MissileWingController right_rear_;
    MissileWingController down_rear_;
    MissileWingController up_rear_;
};

}
