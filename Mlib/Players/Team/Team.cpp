#include "Team.hpp"
#include <Mlib/Physics/Advance_Times/Bullet.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Game_Logic/Game_Statistics.hpp>
#include <stdexcept>

using namespace Mlib;

Team::Team(
    NTeamCountType id,
    VariableAndHash<std::string> name,
    GameStatistics& game_statistics)
    : id_{ id }
    , name_{ std::move(name) }
    , game_statistics_{ game_statistics }
    , nwins_{ 0 }
    , nlosses_{ 0 }
    , destruction_observers_{ *this }
{}

Team::~Team() {
    on_destroy.clear();
    destruction_observers_.clear();
}

NTeamCountType Team::id() const {
    return id_;
}

const VariableAndHash<std::string>& Team::name() const {
    return name_;
}

void Team::notify_kill(RigidBodyVehicle& rigid_body_vehicle) {
    for (const auto& [_, iplayer] : rigid_body_vehicle.drivers_.players_map()) {
        auto* player = dynamic_cast<Player*>(&iplayer.get());
        if (player == nullptr) {
            throw std::runtime_error("Driver is not a player");
        }
        if (&player->team().get() != this) {
            game_statistics_.notify_local_kill(
                nullptr,
                player,
                this,
                &player->team().get(),
                rigid_body_vehicle);
        }
    }
}

DestructionFunctions& Team::on_destroy_team() {
    return on_destroy.deflt;
}

void Team::add_player(const VariableAndHash<std::string>& player) {
    if (!players_.insert(player).second) {
        throw std::runtime_error("Team already contains a player with name \"" + *player + '"');
    }
}

const std::set<VariableAndHash<std::string>>& Team::players() const {
    return players_;
}

uint32_t Team::nwins() const {
    return nwins_;
}

uint32_t Team::nlosses() const {
    return nlosses_;
}

uint32_t Team::nkills() const {
    return game_statistics_.nkills_team(id_);
}

void Team::increase_nwins() {
    ++nwins_;
}

void Team::increase_nlosses() {
    ++nlosses_;
}
