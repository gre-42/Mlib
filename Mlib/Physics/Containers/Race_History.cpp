#include "Race_History.hpp"
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Env.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Json/Misc.hpp>
#include <Mlib/Macro_Executor/Translator.hpp>
#include <Mlib/Physics/Containers/Race_Configuration.hpp>
#include <Mlib/Physics/Containers/Race_Identifier.hpp>
#include <Mlib/Physics/Containers/Race_State.hpp>
#include <Mlib/Physics/Misc/Track_Element.hpp>
#include <Mlib/Physics/Misc/Track_Writer.hpp>
#include <Mlib/Physics/Score_Board_Configuration.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Time/Format.hpp>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <sstream>

namespace fs = std::filesystem;

using json = nlohmann::json;

using namespace Mlib;

static const auto WORLD = VariableAndHash<std::string>{"world"};

namespace Mlib {

void to_json(nlohmann::json& j, const LapTimeEventAndId& l) {
    j["id"] = l.id;
    j["lap_times_seconds"] = l.lap_times_seconds;
    j["playback_exists"] = l.playback_exists;
    j["race_time_seconds"] = l.event.race_time_seconds;
    j["player_name"] = l.event.player_name;
    j["vehicle"] = l.event.vehicle;
    j["vehicle_colors"] = l.event.vehicle_colors;
}

void from_json(const nlohmann::json& j, LapTimeEventAndId& l) {
    l.id = j.at("id").get<size_t>();
    l.lap_times_seconds = j.at("lap_times_seconds").get<std::list<float>>();
    l.playback_exists = j.contains("playback_exists")
        ? j.at("playback_exists").get<bool>()
        : true;
    l.event.race_time_seconds = j.at("race_time_seconds").get<float>();
    l.event.player_name = j.at("player_name").get<std::string>();
    l.event.vehicle = j.at("vehicle").get<std::string>();
    l.event.vehicle_colors = j.at("vehicle_colors").get<UUVector<FixedArray<float, 3>>>();
}

void to_json(nlohmann::json& j, const RaceConfiguration& c) {
    j["readonly"] = c.readonly;
}

void from_json(const nlohmann::json& j, RaceConfiguration& c) {
    c.readonly = j.at("readonly").get<bool>();
}

}

static void save_json(
    const json& j,
    const std::string& dst_filename,
    const std::string& tmp_filename)
{
    {
        auto fstr = create_ofstream(tmp_filename);
        *fstr << j.dump(4);
        fstr->flush();
        if (fstr->fail()) {
            THROW_OR_ABORT("Could not save temporary file \"" + tmp_filename + '"');
        }
    }
    if (path_exists(dst_filename)) {
        remove_path(dst_filename);
    }
    rename_path(tmp_filename, dst_filename);
}

RaceHistory::RaceHistory(
    size_t max_tracks,
    bool save_playback,
    const SceneNodeResources& scene_node_resources,
    const RaceIdentifier& race_identifier,
    std::shared_ptr<Translator> translator)
    : max_tracks_{ max_tracks }
    , save_playback_{ save_playback }
    , scene_node_resources_{ scene_node_resources }
    , translator_{ std::move(translator) }
{
    if (!race_identifier.session.empty()) {
        set_race_identifier_and_reload(race_identifier);
    }
}

RaceHistory::~RaceHistory() = default;

std::string RaceHistory::race_dirname() const {
    std::shared_lock lock{ mutex_ };
    return get_path_in_appdata_directory({race_identifier_.dirname()});
}

std::string RaceHistory::stats_json_filename() const {
    std::shared_lock lock{ mutex_ };
    return (fs::path{race_dirname()} / "stats.json").string();
}

std::string RaceHistory::config_json_filename() const {
    std::shared_lock lock{ mutex_ };
    return (fs::path{race_dirname()} / "config.json").string();
}

std::string RaceHistory::track_m_filename(size_t id) const {
    std::shared_lock lock{ mutex_ };
    return (fs::path{race_dirname()} / ("track_" + std::to_string(id) + ".m")).string();
}

void RaceHistory::set_race_identifier_and_reload(const RaceIdentifier& race_identifier) {
    std::scoped_lock lock{ mutex_ };

    lap_time_events_.clear();
    race_identifier_ = race_identifier;

    std::string fn = stats_json_filename();
    if (path_exists(fn)) {
        auto fstr = create_ifstream(fn);
        json j;
        try {
            *fstr >> j;
        } catch (const nlohmann::detail::parse_error& p) {
            throw std::runtime_error("Could not parse file \"" + fn + "\": " + p.what());
        }
        if (fstr->fail()) {
            THROW_OR_ABORT("Could not load \"" + fn + '"');
        }
        try {
            lap_time_events_ = j.get<std::list<LapTimeEventAndId>>();
        } catch (const nlohmann::detail::type_error& e) {
            throw std::runtime_error("Could not parse " + fn + ": " + e.what());
        }
    }
}

void RaceHistory::start_race(const RaceConfiguration& race_configuration) {
    {
        std::string dn = race_dirname();
        bool dn_exists = path_exists(dn);
        if (!dn_exists) {
            create_directories(dn);
        }
    }
    {
        std::string cn = config_json_filename();
        if (!path_exists(cn)) {
            save_json(json(race_configuration), cn, cn + "~");
        } else {
            auto fstr = create_ifstream(cn);
            json j;
            try {
                *fstr >> j;
            } catch (const nlohmann::detail::parse_error& e) {
                throw std::runtime_error("Could not parse file \"" + cn + "\": " + e.what());
            }
            try {
                if (j.get<RaceConfiguration>().readonly) {
                    THROW_OR_ABORT("Attempt to restart readonly race");
                }
            } catch (const nlohmann::json::exception& e) {
                throw std::runtime_error("Error in file \"" + cn + "\": " + e.what());
            }
        }
    }
}

void RaceHistory::save_and_discard() {
    std::scoped_lock lock{ mutex_ };
    {
        size_t ntracks = 0;
        lap_time_events_.remove_if([&ntracks, this](const LapTimeEventAndId& l){
            if (ntracks < max_tracks_) {
                ++ntracks;
                return false;
            } else {
                auto fn = track_m_filename(l.id);
                if (l.playback_exists) {
                    remove_path(fn);
                } else if (path_exists(fn)) {
                    THROW_OR_ABORT("Did not expect playback \"" + fn + "\" to exist");
                }
                return true;
            }
        });
    }
    {
        std::string old_json_filename = stats_json_filename();
        std::string new_json_filename = old_json_filename + "~";
        save_json(json(lap_time_events_), old_json_filename, new_json_filename);
    }
}

RaceState RaceHistory::notify_lap_finished(
    const LapTimeEvent& lap_time_event,
    const std::list<float>& lap_times_seconds,
    const std::list<TrackElement>& track)
{
    std::scoped_lock lock{ mutex_ };
    if (lap_times_seconds.size() > race_identifier_.laps) {
        THROW_OR_ABORT(
            "Counted number of laps is " +
            std::to_string(lap_times_seconds.size()) +
            ", but race only consists of " +
            std::to_string(race_identifier_.laps) +
            " laps");
    }
    if (lap_times_seconds.size() < race_identifier_.laps) {
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
    if (save_playback_) {
        TrackWriter track_writer{
            track_m_filename(max_id),
            scene_node_resources_.get_geographic_mapping(WORLD) };
        for (const auto& e : track) {
            track_writer.write(e);
        }
        track_writer.flush();
    }
    LapTimeEventAndId lid{
        .event = lap_time_event,
        .id = max_id,
        .lap_times_seconds = lap_times_seconds,
        .playback_exists = save_playback_
    };
    // From: https://stackoverflow.com/a/35840954/2292832
    lap_time_events_.insert(
        std::lower_bound(
            lap_time_events_.begin(),
            lap_time_events_.end(),
            lid,
            [](const LapTimeEventAndId& a, const LapTimeEventAndId& b){
                return a.lap_times_seconds < b.lap_times_seconds;
            }),
        lid);
    save_and_discard();
    return RaceState::FINISHED;
}

uint32_t RaceHistory::rank(float race_time_seconds) const {
    uint32_t rank = 0;
    std::shared_lock guard{ mutex_ };
    for (const auto& l : lap_time_events_) {
        if (race_time_seconds <= l.event.race_time_seconds) {
            break;
        }
        if (rank == UINT32_MAX) {
            THROW_OR_ABORT("Too many ranks");
        }
        ++rank;
    }
    return rank;
}

std::string RaceHistory::get_level_history(ScoreBoardConfiguration score_board_config) const {
    std::stringstream sstr;
    {
        static const VariableAndHash<std::string> race_time{ "race_time" };
        static const VariableAndHash<std::string> Race_time{ "Race_time" };
        std::shared_lock guard{ mutex_ };
        for (const auto& [rank, l] : enumerate(lap_time_events_)) {
            if (any(score_board_config & ScoreBoardConfiguration::PLAYER)) {
                sstr << (rank + 1) << ": " << l.event.player_name << ", " << translator_->translate(race_time) << ": " << format_minutes_seconds(l.event.race_time_seconds) << std::endl;
            } else {
                sstr << (rank + 1) << ": " << translator_->translate(Race_time) << ": " << format_minutes_seconds(l.event.race_time_seconds) << std::endl;
            }
        }
    }
    return sstr.str();
}

std::optional<LapTimeEventAndIdAndMfilename> RaceHistory::get_winner_track_filename(size_t rank) const {
    std::shared_lock guard{ mutex_ };
    for (const auto& [i, l] : enumerate(lap_time_events_)) {
        if (i == rank) {
            if (!l.playback_exists) {
                break;
            }
            return LapTimeEventAndIdAndMfilename{
                .event = l.event,
                .m_filename = track_m_filename(l.id)
            };
        }
    }
    return std::nullopt;
}

const RaceIdentifier& RaceHistory::race_identifier() const {
    return race_identifier_;
}
