#include "Game_History.hpp"
#include <Mlib/Env.hpp>
#include <Mlib/Time.hpp>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>

namespace fs = std::filesystem;
using json = nlohmann::json;

using namespace Mlib;

GameHistory::GameHistory()
{
    load();
}

GameHistory::~GameHistory()
{}

std::string GameHistory::filename() const {
    return get_path_in_home_directory(".game_stats.json");;
}

void GameHistory::load() {
    std::string fn = filename();
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
                l["level"].get<std::string>(),
                l["player_name"].get<std::string>(),
                l["lap_time"].get<float>()});
        }
    }
}

void GameHistory::save() const {
    std::string fn = filename();
    std::ofstream fstr{fn.c_str()};
    json j;
    size_t i = 0;
    for (const auto& l : lap_time_events_) {
        j[i]["level"] = l.level;
        j[i]["player_name"] = l.player_name;
        j[i]["lap_time"] = l.lap_time;
        ++i;
    }
    fstr << j;
    if (fstr.fail()) {
        throw std::runtime_error("Could not save \"" + fn + '"');
    }
}

void GameHistory::notify_lap_time(const LapTimeEvent& lap_time_event) {
    lap_time_events_.push_front(lap_time_event);
    save();
}

std::string GameHistory::get_level_history(const std::string& level) const {
    std::stringstream sstr;
    for (const auto& l : lap_time_events_) {
        if (l.level == level) {
            sstr << "Player: " << l.player_name << ", lap time: " << format_minutes_seconds(l.lap_time) << std::endl;
        }
    }
    return sstr.str();
}
