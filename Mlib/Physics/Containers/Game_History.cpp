#include "Game_History.hpp"
#include <Mlib/Env.hpp>
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

GameHistory::GameHistory(
    size_t max_tracks,
    const SceneNodeResources& scene_node_resources)
: max_tracks_{max_tracks},
  scene_node_resources_{scene_node_resources}
{
    set_session_name_and_reload("session1");
}

GameHistory::~GameHistory()
{}

std::string GameHistory::config_dirname() const {
    std::shared_lock lock{ mutex_ };
    return get_path_in_home_directory({".osm_rally", session_name_});
}

std::string GameHistory::stats_json_filename() const {
    std::shared_lock lock{ mutex_ };
    return fs::path{config_dirname()} / "stats.json";
}

std::string GameHistory::track_m_filename(size_t id) const {
    std::shared_lock lock{ mutex_ };
    return fs::path{config_dirname()} / ("track_" + std::to_string(id) + ".m");
}

void GameHistory::set_session_name_and_reload(const std::string& session_name) {
    std::unique_lock lock{ mutex_ };

    lap_time_events_.clear();
    session_name_ = session_name;

    {
        std::string dn = config_dirname();
        try {
            fs::create_directories(dn);
        } catch (const fs::filesystem_error& e) {
            throw std::runtime_error("Could not create directory \"" + dn + "\". " + e.what());
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
                        .lap_time = l["lap_time"].get<float>(),
                        .player_name = l["player_name"].get<std::string>(),
                        .vehicle = l["vehicle"].get<std::string>(),
                        .vehicle_color = OrderableFixedArray<float, 3>(vehicle_color)},
                    .id = l["id"].get<size_t>() });
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
                entry["lap_time"] = l.event.lap_time;
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
        {
            std::ofstream fstr{new_json_filename.c_str()};
            fstr << j.dump(4);
            fstr.flush();
            if (fstr.fail()) {
                throw std::runtime_error("Could not save \"" + new_json_filename + '"');
            }
        }
        if (fs::exists(old_json_filename)) {
            try {
                fs::remove(old_json_filename);
            } catch (const fs::filesystem_error& e) {
                throw std::runtime_error("Could not delete file \"" + old_json_filename + '"');
            }
        }
        try {
            fs::rename(new_json_filename, old_json_filename);
        } catch (const fs::filesystem_error& e) {
            throw std::runtime_error("Could not rename file \"" + new_json_filename + '"');
        }
    }
}

void GameHistory::notify_lap_time(
    const LapTimeEvent& lap_time_event,
    const std::list<TrackElement>& track)
{
    std::unique_lock lock{ mutex_ };
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
    LapTimeEventAndId lid{lap_time_event, max_id};
    // From: https://stackoverflow.com/a/35840954/2292832
    lap_time_events_.insert(std::lower_bound(lap_time_events_.begin(), lap_time_events_.end(), lid), lid);
    save_and_discard();
}

std::string GameHistory::get_level_history(const std::string& level) const {
    std::stringstream sstr;
    {
        std::shared_lock guard{ mutex_ };
        for (const auto& l : lap_time_events_) {
            if (l.event.level == level) {
                sstr << "Player: " << l.event.player_name << ", lap time: " << format_minutes_seconds(l.event.lap_time) << std::endl;
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
