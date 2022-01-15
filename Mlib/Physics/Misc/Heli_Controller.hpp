#pragma once
#include <Mlib/Physics/Misc/Rigid_Body_Vehicle_Controller.hpp>
#include <cstddef>
#include <map>

namespace Mlib {

class HeliController: public RigidBodyVehicleController {
public:
    explicit HeliController(
        RigidBodyVehicle* rb,
        const std::map<size_t, float>& tire_angles);
    virtual ~HeliController() override;
    virtual void apply() override;
private:
    std::map<size_t, float> tire_angles_;
};

}
