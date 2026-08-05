#include "Scene_Level.hpp"
#include <Mlib/Remote/Remote_Role.hpp>
#include <Mlib/Remote/Session_Id.hpp>
#include <mutex>
#include <stdexcept>

using namespace Mlib;

LocalSceneLevel::LocalSceneLevel()
    : LocalSceneLevel{"", "", get_session_id(0)}
{}

LocalSceneLevel::LocalSceneLevel(
    std::string level_name,
    std::string time_of_day,
    ReloadCountType reload_count)
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
    std::function<void()> on_reload_transient_objects,
    std::function<void()> on_update_time_of_day)
    : load_status_{ LocalSceneLevelLoadStatus::LOADING }
    , local_scene_level_{ std::move(local_scene_level) }
    , on_schedule_load_scene_{ std::move(on_schedule_load_scene) }
    , on_reload_transient_objects_{ std::move(on_reload_transient_objects) }
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

std::string SceneLevelSelector::get_next_time_of_day() const {
    std::shared_lock lock{ mutex_ };
    return next_scene_level_.time_of_day;
}

bool SceneLevelSelector::server_set_next_scene_level(
    const std::string& level_name,
    const std::string& time_of_day)
{
    std::scoped_lock lock{ mutex_ };
    linfo() << "Server set next scene level";
    if (local_scene_level_.level_name == level_name) {
        return set_next_scene_level(level_name, time_of_day, local_scene_level_.reload_count, RemoteRole::SERVER);
    } else {
        return set_next_scene_level(level_name, time_of_day, get_session_id(local_scene_level_.reload_count), RemoteRole::SERVER);
    }
}

bool SceneLevelSelector::client_set_next_scene_level(
    const std::string& level_name,
    const std::string& time_of_day,
    ReloadCountType reload_count)
{
    return set_next_scene_level(level_name, time_of_day, reload_count, RemoteRole::CLIENT);
}

bool SceneLevelSelector::set_next_scene_level(
    const std::string& level_name,
    const std::string& time_of_day,
    ReloadCountType reload_count,
    RemoteRole remote_role)
{
    std::scoped_lock lock{ mutex_ };
    auto print = [&](const std::string& prefix){
        linfo() << prefix << "\"" << level_name <<
            "\", time of day: \"" << time_of_day <<
            "\", reload count: " << (reload_count + 0) <<
            ", current level: \"" << local_scene_level_.level_name << '"';
    };
    if (!on_schedule_load_scene_) {
        throw std::runtime_error("on_schedule_load_scene not set");
    }
    if (!on_reload_transient_objects_) {
        throw std::runtime_error("on_reload_transient_objects not set");
    }
    if (!on_update_time_of_day_) {
        throw std::runtime_error("on_update_time_of_day not set in server");
    }
    if (load_status_ == LocalSceneLevelLoadStatus::LOADING) {
        return true;
    }
    if ((local_scene_level_.level_name != level_name) ||
        (local_scene_level_.reload_count != reload_count))
    {
        print("Load scene: ");
        load_status_ = LocalSceneLevelLoadStatus::LOADING;
        next_scene_level_.level_name = level_name;
        next_scene_level_.time_of_day = time_of_day;
        next_scene_level_.reload_count = reload_count;
        on_schedule_load_scene_();
        return true;
    } else if (local_scene_level_.time_of_day != time_of_day) {
        load_status_ = LocalSceneLevelLoadStatus::LOADING;
        next_scene_level_.time_of_day = time_of_day;
        if (remote_role == RemoteRole::SERVER) {
            print("Update time of day as server: ");
            on_reload_transient_objects_();
        } else {
            print("Update time of day as client: ");
            on_update_time_of_day_();
        }
        local_scene_level_.time_of_day = time_of_day;
        load_status_ = LocalSceneLevelLoadStatus::RUNNING;
    } else if (remote_role == RemoteRole::SERVER) {
        load_status_ = LocalSceneLevelLoadStatus::LOADING;
        print("Reload transient objects: ");
        on_reload_transient_objects_();
        load_status_ = LocalSceneLevelLoadStatus::RUNNING;
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
