#include "Scene_Level.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

LocalSceneLevel::LocalSceneLevel()
    : LocalSceneLevel{"", 0}
{}

LocalSceneLevel::LocalSceneLevel(
    std::string name,
    uint8_t reload_count)
    : name{ std::move(name) }
    , reload_count{ reload_count }
{}

LocalSceneLevel::~LocalSceneLevel() = default;

bool LocalSceneLevel::reload_required(const LocalSceneLevel& other_level) const
{
    return
        (reload_count != other_level.reload_count) ||
        (name != other_level.name);
}

SceneLevelSelector::SceneLevelSelector(
    LocalSceneLevel local_scene_level,
    std::function<void()> on_schedule_load_scene)
    : load_status_{ LocalSceneLevelLoadStatus::LOADING }
    , local_scene_level_{ std::move(local_scene_level) }
    , on_schedule_load_scene_{ std::move(on_schedule_load_scene) }
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
    return next_scene_level_.name;
}

void SceneLevelSelector::server_set_next_scene_level(std::string level) {
    std::scoped_lock lock{ mutex_ };
    next_scene_level_.name = std::move(level);
    next_scene_level_.reload_count = local_scene_level_.reload_count + 1;
}

void SceneLevelSelector::client_schedule_load_scene_level(LocalSceneLevel level) {
    std::scoped_lock lock{ mutex_ };
    if (!on_schedule_load_scene_) {
        THROW_OR_ABORT("on_schedule_load_scene not set");
    }
    next_scene_level_ = std::move(level);
    if (on_schedule_load_scene_) {
        on_schedule_load_scene_();
    }
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
