#include "Scene_Level.hpp"
#include <mutex>
#include <stdexcept>

using namespace Mlib;

LocalSceneLevel::LocalSceneLevel()
    : LocalSceneLevel{"", "", 0}
{}

LocalSceneLevel::LocalSceneLevel(
    std::string level_name,
    std::string time_of_day,
    uint8_t reload_count)
    : level_name{ std::move(level_name) }
    , time_of_day{ std::move(time_of_day) }
    , reload_count{ reload_count }
{}

LocalSceneLevel::~LocalSceneLevel() = default;

bool LocalSceneLevel::reload_required(const LocalSceneLevel& other_level) const
{
    return
        (reload_count != other_level.reload_count) ||
        (level_name != other_level.level_name) ||
        (time_of_day != other_level.time_of_day);
}

SceneLevelSelector::SceneLevelSelector(
    LocalSceneLevel local_scene_level,
    std::function<void()> on_schedule_load_scene,
    std::function<void()> on_update_time_of_day)
    : load_status_{ LocalSceneLevelLoadStatus::LOADING }
    , local_scene_level_{ std::move(local_scene_level) }
    , on_schedule_load_scene_{ std::move(on_schedule_load_scene) }
    , on_update_time_of_day_{ std::move(on_update_time_of_day) }
{}

SceneLevelSelector::~SceneLevelSelector() = default;

LocalSceneLevel SceneLevelSelector::get_local_scene_level() const {
    std::shared_lock lock{ mutex_ };
    return local_scene_level_;
}

LocalSceneLevel SceneLevelSelector::get_next_scene_level() const {
    std::shared_lock lock{ mutex_ };
    return next_scene_level_;
}

std::string SceneLevelSelector::get_next_scene_name() const {
    std::shared_lock lock{ mutex_ };
    return next_scene_level_.level_name;
}

std::string SceneLevelSelector::get_time_of_day() const {
    std::shared_lock lock{ mutex_ };
    return next_scene_level_.time_of_day;
}

bool SceneLevelSelector::server_set_next_scene_level(
    const std::string& level_name,
    const std::string& time_of_day)
{
    std::scoped_lock lock{ mutex_ };
    if (local_scene_level_.level_name == level_name) {
        return set_next_scene_level(level_name, time_of_day, local_scene_level_.reload_count);
    } else {
        return set_next_scene_level(level_name, time_of_day, local_scene_level_.reload_count + 1);
    }
}

bool SceneLevelSelector::client_set_next_scene_level(
    const std::string& level_name,
    const std::string& time_of_day,
    uint8_t reload_count)
{
    return set_next_scene_level(level_name, time_of_day, reload_count);
}

bool SceneLevelSelector::set_next_scene_level(
    const std::string& level_name,
    const std::string& time_of_day,
    uint8_t reload_count)
{
    std::scoped_lock lock{ mutex_ };
    if (!on_schedule_load_scene_) {
        throw std::runtime_error("on_schedule_load_scene not set in server");
    }
    if (!on_update_time_of_day_) {
        throw std::runtime_error("on_update_time_of_day not set in server");
    }
    if ((local_scene_level_.level_name != level_name) ||
        (local_scene_level_.reload_count != reload_count))
    {
        next_scene_level_.level_name = level_name;
        next_scene_level_.time_of_day = time_of_day;
        next_scene_level_.reload_count = reload_count;
        if (on_schedule_load_scene_) {
            on_schedule_load_scene_();
        }
        return true;
    } else if (local_scene_level_.time_of_day != time_of_day) {
        local_scene_level_.time_of_day = time_of_day;
        local_scene_level_.reload_count = reload_count;
        if (on_update_time_of_day_) {
            on_update_time_of_day_();
        }
    }
    return false;
}

bool SceneLevelSelector::reload_required(const LocalSceneLevel& other_level) const {
    std::shared_lock lock{ mutex_ };
    return local_scene_level_.reload_required(other_level);
}

LocalSceneLevelLoadStatus SceneLevelSelector::load_status() const {
    std::shared_lock lock{ mutex_ };
    return load_status_;
}

void SceneLevelSelector::notify_level_loaded() {
    std::scoped_lock lock{ mutex_ };
    load_status_ = LocalSceneLevelLoadStatus::RUNNING;
}
