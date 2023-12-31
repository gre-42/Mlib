#pragma once
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {

inline Player& get_driver(RigidBodyVehicle& rb) {
    if (rb.driver_ == nullptr) {
        THROW_OR_ABORT("Rigid body has no driver");
    }
    auto player = dynamic_cast<Player*>(rb.driver_);
    if (player == nullptr) {
        THROW_OR_ABORT("Driver is not player");
    }
    return *player;
}

}
