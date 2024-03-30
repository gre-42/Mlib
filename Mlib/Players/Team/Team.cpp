#include "Team.hpp"
#include <Mlib/Physics/Advance_Times/Bullet.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

Team::Team()
: destruction_observers{*this},
  nwins_{0},
  nlosses_{0},
  nkills_{0}
{}

Team::~Team() {
    destruction_observers.shutdown();
}

void Team::notify_kill(RigidBodyVehicle& rigid_body_vehicle) {
    if (rigid_body_vehicle.driver_ == nullptr) {
        return;
    }
    Player* player = dynamic_cast<Player*>(rigid_body_vehicle.driver_);
    if (player == nullptr) {
        THROW_OR_ABORT("Driver is not a player");
    }
    if (&player->team() != this) {
        ++nkills_;
    }
}

void Team::notify_bullet_destroyed(Bullet& bullet) {
    destruction_observers.remove({ bullet, CURRENT_SOURCE_LOCATION });
}

void Team::add_player(const std::string& player) {
    if (!players_.insert(player).second) {
        THROW_OR_ABORT("Team already contains a player with name \"" + player + '"');
    }
}

const std::set<std::string>& Team::players() const {
    return players_;
}

uint32_t Team::nwins() const {
    return nwins_;
}

uint32_t Team::nlosses() const {
    return nlosses_;
}

uint32_t Team::nkills() const {
    return nkills_;
}

void Team::increase_nwins() {
    ++nwins_;
}

void Team::increase_nlosses() {
    ++nlosses_;
}

void Team::increase_nkills() {
    ++nkills_;
}
