#include "Game_History.hpp"
#include <Mlib/Env.hpp>
#include <Mlib/Physics/Misc/Track_Element.hpp>
#include <Mlib/Time.hpp>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <Mlib/Physics/Misc/Track_Writer.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

using namespace Mlib;

GameHistory::GameHistory(size_t max_tracks)
: max_tracks_{max_tracks}
{
    fs::create_directory(config_dirname());
    load();
}

GameHistory::~GameHistory()
{}

std::string GameHistory::config_dirname() const {
    return get_path_in_home_directory({".osm_rally"});
}

std::string GameHistory::stats_json_filename() const {
    return get_path_in_home_directory({config_dirname(), "stats.json"});
}

std::string GameHistory::track_m_filename(size_t id) const {
    return get_path_in_home_directory({config_dirname(), "track_" + std::to_string(id) + ".m"});
}

void GameHistory::load() {
    std::string fn = stats_json_filename();
    if (fs::exists(fn)) {
        std::ifstream fstr{fn.c_str()};
        json j;
        try {
            fstr >> j;
        } catch (const nlohmann::detail::parse_error& p) {
            throw std::runtime_error("Could not parse file \"" + fn + "\": " + p.what());
        }
        if (fstr.fail()) {
            throw std::runtime_error("Could not load \"" + fn + '"');
        }
        for (const auto& l : j) {
            lap_time_events_.push_back({
                .event{
                    .level = l["level"].get<std::string>(),
                    .lap_time = l["lap_time"].get<float>(),
                    .player_name = l["player_name"].get<std::string>()},
                .id = l["id"].get<size_t>()});
        }
    }
}

void GameHistory::save_and_discard() {
    std::string fn = stats_json_filename();
    std::ofstream fstr{fn.c_str()};
    json j;
    std::map<std::string, size_t> ntracks;
    lap_time_events_.remove_if([&ntracks, &j, this](const LapTimeEventAndId& l){
        size_t& i = ntracks[l.event.level];
        if (i < max_tracks_) {
            json entry;
            entry["id"] = l.id;
            entry["level"] = l.event.level;
            entry["lap_time"] = l.event.lap_time;
            entry["player_name"] = l.event.player_name;
            j.push_back(entry);
            ++i;
            return false;
        } else {
            fs::remove(track_m_filename(l.id));
            return true;
        }
    });
    fstr << j.dump(4);
    if (fstr.fail()) {
        throw std::runtime_error("Could not save \"" + fn + '"');
    }
}

void GameHistory::notify_lap_time(
    const LapTimeEvent& lap_time_event,
    const std::list<TrackElement>& track)
{
    size_t max_id;
    auto max_element = std::max_element(
        lap_time_events_.begin(),
        lap_time_events_.end(),
        [](const LapTimeEventAndId& a, const LapTimeEventAndId& b){return a.id < b.id;});
    if (max_element == lap_time_events_.end()) {
        max_id = 0;
    } else {
        max_id = max_element->id + 1;
    }
    TrackWriter track_writer{track_m_filename(max_id)};
    for (const auto& e : track) {
        track_writer.write(e);
    }
    LapTimeEventAndId lid{lap_time_event, max_id};
    // From: https://stackoverflow.com/a/35840954/2292832
    lap_time_events_.insert(std::lower_bound(lap_time_events_.begin(), lap_time_events_.end(), lid), lid);
    save_and_discard();
}

std::string GameHistory::get_level_history(const std::string& level) const {
    std::stringstream sstr;
    for (const auto& l : lap_time_events_) {
        if (l.event.level == level) {
            sstr << "Player: " << l.event.player_name << ", lap time: " << format_minutes_seconds(l.event.lap_time) << std::endl;
        }
    }
    return sstr.str();
}

std::string GameHistory::get_winner_track_filename(const std::string& level, size_t rank) const {
    size_t i = 0;
    for (const auto& l : lap_time_events_) {
        if (l.event.level != level) {
            continue;
        }
        if (i == rank) {
            return track_m_filename(l.id);
        }
        ++i;
    }
    return "";
}
