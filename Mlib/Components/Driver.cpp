#include "Driver.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

DanglingBaseClassRef<Player> Mlib::get_driver(RigidBodyVehicle& rb) {
    if (rb.driver_ == nullptr) {
        THROW_OR_ABORT("Rigid body has no driver");
    }
    auto player = dynamic_cast<Player*>(rb.driver_.get());
    if (player == nullptr) {
        THROW_OR_ABORT("Driver is not player");
    }
    return { *player, CURRENT_SOURCE_LOCATION };
}
