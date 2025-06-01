#include "Avatar_Movement.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Avatar_Controllers/Rigid_Body_Avatar_Controller.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

AvatarMovement::AvatarMovement(Player& player)
    : player_{ player }
{}

void AvatarMovement::run_move(
    float yaw,
    float pitch,
    float forwardmove,
    float sidemove)
{
    player_.delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!player_.has_scene_vehicle()) {
        THROW_OR_ABORT("run_move despite rigid body nullptr");
    }

    auto& ac = player_.rigid_body()->avatar_controller();
    ac.reset();

    ac.set_target_yaw(yaw);
    ac.set_target_pitch(pitch);

    FixedArray<float, 3> direction{ sidemove, 0.f, -forwardmove };
    float len2 = sum(squared(direction));
    if (len2 < 1e-12) {
        ac.stop();
    } else {
        float len = std::sqrt(len2);
        ac.increment_legs_z(direction / len);
        ac.walk(player_.vehicle_movement.surface_power_forward(), 1.f);
    }
    ac.apply();
}
