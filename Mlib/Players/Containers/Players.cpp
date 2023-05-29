#include "Players.hpp"
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Containers/Race_History.hpp>
#include <Mlib/Physics/Containers/Race_Identifier.hpp>
#include <Mlib/Physics/Score_Board_Configuration.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Team/Team.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Time/Format.hpp>
#include <filesystem>

namespace fs = std::filesystem;

using namespace Mlib;

Players::Players(
    AdvanceTimes& advance_times,
    const std::string& level_name,
    size_t max_tracks,
    const SceneNodeResources& scene_node_resources,
    const RaceIdentifier& race_identifier)
: advance_times_{advance_times},
  race_history_{std::make_unique<RaceHistory>(
    max_tracks,
    scene_node_resources,
    race_identifier)}
{}

Players::~Players()
{
    for (const auto& [_, p] : players_) {
        advance_times_.delete_advance_time(*p);
    }
}

void Players::add_player(std::unique_ptr<Player>&& player) {
    std::string player_name = player->name();
    Player* p = player.get();
    if (!players_.insert(std::make_pair(player->name(), std::move(player))).second) {
        THROW_OR_ABORT("Player with name \"" + player_name + "\" already exists");
    }
    if (!teams_.contains(p->team_name())) {
        if (!teams_.insert({p->team_name(), std::make_unique<Team>()}).second) {
            THROW_OR_ABORT("Could not insert team");
        }
    }
    teams_.at(p->team_name())->add_player(p->name());
}

Player& Players::get_player(const std::string& name) {
    auto it = players_.find(name);
    if (it == players_.end()) {
        THROW_OR_ABORT("No player with name \"" + name + "\" exists");
    }
    return *it->second;
}

const Player& Players::get_player(const std::string& name) const {
    return const_cast<Players*>(this)->get_player(name);
}

Team& Players::get_team(const std::string& name) {
    auto it = teams_.find(name);
    if (it == teams_.end()) {
        THROW_OR_ABORT("No team with name \"" + name + "\" exists");
    }
    return *it->second;
}

const Team& Players::get_team(const std::string& name) const {
    return const_cast<Players*>(this)->get_team(name);
}

void Players::set_team_waypoint(const std::string& team_name, const FixedArray<double, 3>& waypoint) {
    for (auto& p : players_) {
        if (p.second->team_name() == team_name) {
            p.second->single_waypoint().set_waypoint(waypoint);
        }
    }
}

const RaceIdentifier& Players::race_identifier() const {
    return race_history_->race_identifier();
}

void Players::set_race_identifier_and_reload_history(const RaceIdentifier& race_identifier) {
    race_history_->set_race_identifier_and_reload(race_identifier);
}

void Players::start_race(const RaceConfiguration& race_configuration) {
    race_history_->start_race(race_configuration);
}

RaceState Players::notify_lap_finished(
    const Player* player,
    float race_time_seconds,
    const std::list<float>& lap_times_seconds,
    const std::list<TrackElement>& track)
{
    return race_history_->notify_lap_finished({
        .race_time_seconds = race_time_seconds,
        .player_name = player->name(),
        .vehicle = player->vehicle_name(),
        .vehicle_color = OrderableFixedArray{player->vehicle_color()}},
        lap_times_seconds,
        track);
}

uint32_t Players::rank(float race_time_seconds) const {
    return race_history_->rank(race_time_seconds);
}

LapTimeEventAndIdAndMfilename Players::get_winner_track_filename(size_t rank) const {
    return race_history_->get_winner_track_filename(rank);
}

std::string Players::get_score_board(ScoreBoardConfiguration config) const {
    std::stringstream sstr;
    for (const auto& [tname, team] : teams_) {
        sstr << "Team: " << tname;
        if (any(config & ScoreBoardConfiguration::NWINS)) {
            sstr << ", wins: " << team->nwins();
        }
        if (any(config & ScoreBoardConfiguration::NKILLS)) {
            sstr << ", kills: " << team->nkills();
        }
        sstr << std::endl;
        for (const auto& pname : team->players()) {
            const auto& p = get_player(pname);
            if (p.game_mode() == GameMode::BYSTANDER) {
                continue;
            }
            sstr << "Player: " << pname;
            if (any(config & ScoreBoardConfiguration::TEAM)) {
                sstr << ", team: " << p.team_name();
            }
            if (any(config & ScoreBoardConfiguration::BEST_LAP_TIME)) {
                sstr << ", best lap time: " << format_minutes_seconds(p.stats().best_lap_time);
            }
            if (any(config & ScoreBoardConfiguration::RACE_TIME)) {
                if (p.stats().race_time != INFINITY) {
                    sstr << ", race time: " << format_minutes_seconds(p.stats().race_time);
                }
            }
            if (any(config & ScoreBoardConfiguration::LAPS)) {
                if ((race_history_->race_identifier().laps != 1) &&
                    (p.stats().nlaps != race_history_->race_identifier().laps))
                {
                    sstr << ", lap " <<  (p.stats().nlaps + 1) << "/" << race_history_->race_identifier().laps;
                }
            }
            if (any(config & ScoreBoardConfiguration::RANK)) {
                if (p.stats().rank != UINT32_MAX) {
                    sstr << ", rank " <<  (p.stats().rank + 1);
                }
            }
            if (any(config & ScoreBoardConfiguration::CAR_HP)) {
                sstr << ", car HP: " << p.car_health();
            }
            if (any(config & ScoreBoardConfiguration::NWINS)) {
                sstr << ", wins: " << p.stats().nwins;
            }
            if (any(config & ScoreBoardConfiguration::NKILLS)) {
                sstr << ", kills: " << p.stats().nkills;
            }
            sstr << std::endl;
        }
    }
    if (any(config & ScoreBoardConfiguration::HISTORY)) {
        sstr << std::endl;
        sstr << "History" << std::endl;
        sstr << race_history_->get_level_history() << std::endl;
    }
    return sstr.str();
}

std::map<std::string, std::unique_ptr<Player>>& Players::players() {
    return players_;
}

const std::map<std::string, std::unique_ptr<Player>>& Players::players() const {
    return players_;
}

std::map<std::string, std::unique_ptr<Team>>& Players::teams() {
    return teams_;
}

const std::map<std::string, std::unique_ptr<Team>>& Players::teams() const {
    return teams_;
}

size_t Players::nactive() const {
    size_t nactive = 0;
    for (const auto& [_, p] : players_) {
        nactive += !p->scene_node_name().empty();
    }
    return nactive;
}

std::ostream& Mlib::operator << (std::ostream& ostr, const Players& players) {
    for (const auto& p : players.players_) {
        ostr << p.second->name() << '\n';
    }
    return ostr;
}
