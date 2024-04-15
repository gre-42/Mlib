#include "Team.hpp"
#include <Mlib/Physics/Advance_Times/Bullet.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

Team::Team(std::string name)
    : name_{ std::move(name) }
    , nwins_{ 0 }
    , nlosses_{ 0 }
    , nkills_{ 0 }
    , destruction_observers_{ *this }
{}

Team::~Team() {
    on_destroy.clear();
    destruction_observers_.clear();
}

const std::string& Team::name() const {
    return name_;
}

void Team::notify_kill(RigidBodyVehicle& rigid_body_vehicle) {
    if (rigid_body_vehicle.driver_ == nullptr) {
        return;
    }
    Player* player = dynamic_cast<Player*>(rigid_body_vehicle.driver_.get());
    if (player == nullptr) {
        THROW_OR_ABORT("Driver is not a player");
    }
    if (&player->team().get() != this) {
        ++nkills_;
    }
}

DestructionObservers<const ITeam&>& Team::destruction_observers() {
    return destruction_observers_;
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
