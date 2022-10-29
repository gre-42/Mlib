#include "Game_History.hpp"
#include <Mlib/Env.hpp>
#include <Mlib/Physics/Containers/Race_Configuration.hpp>
#include <Mlib/Physics/Containers/Race_State.hpp>
#include <Mlib/Physics/Misc/Track_Element.hpp>
#include <Mlib/Physics/Misc/Track_Writer.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Time.hpp>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <sstream>

namespace fs = std::filesystem;
using json = nlohmann::json;

using namespace Mlib;

static void save_json(
    const json& j,
    const std::string& old_filename,
    const std::string& new_filename)
{
    {
        std::ofstream fstr{new_filename.c_str()};
        fstr << j.dump(4);
        fstr.flush();
        if (fstr.fail()) {
            throw std::runtime_error("Could not save \"" + new_filename + '"');
        }
    }
    if (fs::exists(old_filename)) {
        try {
            fs::remove(old_filename);
        } catch (const fs::filesystem_error& e) {
            throw std::runtime_error("Could not delete file \"" + old_filename + '"');
        }
    }
    try {
        fs::rename(new_filename, old_filename);
    } catch (const fs::filesystem_error& e) {
        throw std::runtime_error("Could not rename file \"" + new_filename + '"');
    }
}

GameHistory::GameHistory(
    size_t max_tracks,
    const SceneNodeResources& scene_node_resources,
    const RaceConfiguration& race_configuration)
: max_tracks_{max_tracks},
  scene_node_resources_{scene_node_resources}
{
    if (!race_configuration.session.empty()) {
        set_race_configuration_and_reload(race_configuration);
    }
}

GameHistory::~GameHistory()
{}

std::string GameHistory::race_dirname() const {
    std::shared_lock lock{ mutex_ };
    return get_path_in_home_directory({".osm_rally", race_configuration_.dirname()});
}

std::string GameHistory::stats_json_filename() const {
    std::shared_lock lock{ mutex_ };
    return (fs::path{race_dirname()} / "stats.json").string();
}

std::string GameHistory::config_json_filename() const {
    std::shared_lock lock{ mutex_ };
    return (fs::path{race_dirname()} / "config.json").string();
}

std::string GameHistory::track_m_filename(size_t id) const {
    std::shared_lock lock{ mutex_ };
    return (fs::path{race_dirname()} / ("track_" + std::to_string(id) + ".m")).string();
}

void GameHistory::set_race_configuration_and_reload(const RaceConfiguration& race_configuration) {
    std::unique_lock lock{ mutex_ };

    lap_time_events_.clear();
    race_configuration_ = race_configuration;

    {
        std::string dn = race_dirname();
        if (!fs::exists(dn)) {
            try {
                fs::create_directories(dn);
            } catch (const fs::filesystem_error& e) {
                throw std::runtime_error("Could not create directory \"" + dn + "\". " + e.what());
            }
        }
        {
            json j;
            j["readonly"] = race_configuration_.readonly;
            {
                std::string old_json_filename = config_json_filename();
                std::string new_json_filename = old_json_filename + "~";
                save_json(j, old_json_filename, new_json_filename);
            }
        }
    }

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
            try {
                auto vehicle_color = l["vehicle_color"].get<std::vector<float>>();
                if (vehicle_color.size() != 3) {
                    throw std::runtime_error("Vehicle color does not have 3 elements");
                }
                lap_time_events_.push_back(LapTimeEventAndId{
                    .event = LapTimeEvent{
                        .level = l["level"].get<std::string>(),
                        .race_time_seconds = l["race_time_seconds"].get<float>(),
                        .player_name = l["player_name"].get<std::string>(),
                        .vehicle = l["vehicle"].get<std::string>(),
                        .vehicle_color = OrderableFixedArray<float, 3>(vehicle_color)},
                    .id = l["id"].get<size_t>(),
                    .lap_times_seconds = l["lap_times_seconds"].get<std::list<float>>()});
            } catch (const nlohmann::detail::type_error& e) {
                throw std::runtime_error("Could not parse " + fn + ": " + e.what());
            }
        }
    }
}

void GameHistory::save_and_discard() {
    std::unique_lock lock{ mutex_ };
    json j;
    {
        std::map<std::string, size_t> ntracks;
        lap_time_events_.remove_if([&ntracks, &j, this](const LapTimeEventAndId& l){
            size_t& i = ntracks[l.event.level];
            if (i < max_tracks_) {
                json entry;
                entry["id"] = l.id;
                entry["level"] = l.event.level;
                entry["race_time_seconds"] = l.event.race_time_seconds;
                entry["lap_times_seconds"] = l.lap_times_seconds;
                entry["player_name"] = l.event.player_name;
                entry["vehicle"] = l.event.vehicle;
                entry["vehicle_color"] = std::vector<float>(l.event.vehicle_color.flat_begin(), l.event.vehicle_color.flat_end());
                j.push_back(entry);
                ++i;
                return false;
            } else {
                std::string fn = track_m_filename(l.id);
                try {
                    fs::remove(fn);
                } catch (const fs::filesystem_error& e) {
                    throw std::runtime_error("Could not delete file \"" + fn + '"');
                }
                return true;
            }
        });
    }
    {
        std::string old_json_filename = stats_json_filename();
        std::string new_json_filename = old_json_filename + "~";
        save_json(j, old_json_filename, new_json_filename);
    }
}

RaceState GameHistory::notify_lap_time(
    const LapTimeEvent& lap_time_event,
    const std::list<float>& lap_times_seconds,
    const std::list<TrackElement>& track)
{
    std::unique_lock lock{ mutex_ };
    if (lap_times_seconds.size() > race_configuration_.laps) {
        throw std::runtime_error(
            "Counted number of laps is " +
            std::to_string(lap_times_seconds.size()) +
            ", but race only consists of " +
            std::to_string(race_configuration_.laps) +
            "laps");
    }
    if (lap_times_seconds.size() < race_configuration_.laps) {
        return RaceState::ONGOING;
    }
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
    {
        TrackWriter track_writer{track_m_filename(max_id), scene_node_resources_.get_geographic_mapping("world")};
        for (const auto& e : track) {
            track_writer.write(e);
        }
        track_writer.flush();
    }
    LapTimeEventAndId lid{lap_time_event, max_id, lap_times_seconds};
    // From: https://stackoverflow.com/a/35840954/2292832
    lap_time_events_.insert(std::lower_bound(lap_time_events_.begin(), lap_time_events_.end(), lid), lid);
    save_and_discard();
    return RaceState::FINISHED;
}

std::string GameHistory::get_level_history(const std::string& level) const {
    std::stringstream sstr;
    {
        std::shared_lock guard{ mutex_ };
        for (const auto& l : lap_time_events_) {
            if (l.event.level == level) {
                sstr << "Player: " << l.event.player_name << ", race time: " << format_minutes_seconds(l.event.race_time_seconds) << std::endl;
            }
        }
    }
    return sstr.str();
}

LapTimeEventAndIdAndMfilename GameHistory::get_winner_track_filename(const std::string& level, size_t rank) const {
    size_t i = 0;
    std::shared_lock guard{ mutex_ };
    for (const auto& l : lap_time_events_) {
        if (l.event.level != level) {
            continue;
        }
        if (i == rank) {
            return LapTimeEventAndIdAndMfilename{
                .event = l.event,
                .m_filename = track_m_filename(l.id)
            };
        }
        ++i;
    }
    return LapTimeEventAndIdAndMfilename();
}
