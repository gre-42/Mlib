#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <functional>
#include <string>

namespace Mlib {

struct LocalSceneLevel {
    LocalSceneLevel();
    LocalSceneLevel(
        std::string level_name,
        std::string time_of_day,
        uint8_t reload_counter = 0);
    ~LocalSceneLevel();
    bool reload_required(const LocalSceneLevel& other_level) const;
    std::string level_name;
    std::string time_of_day;
    uint8_t reload_count;
};

enum class LocalSceneLevelLoadStatus: uint8_t {
    LOADING = 0xCA,
    RUNNING = 0x2B
};

class SceneLevelSelector: public virtual DanglingBaseClass {
public:
    SceneLevelSelector(
        LocalSceneLevel local_scene_level,
        std::function<void()> on_schedule_load_scene,
        std::function<void()> on_update_time_of_day);
    ~SceneLevelSelector();
    LocalSceneLevel get_local_scene_level() const;
    LocalSceneLevel get_next_scene_level() const;
    std::string get_next_scene_name() const;
    std::string get_time_of_day() const;
    bool server_set_next_scene_level(
        const std::string& level_name,
        const std::string& time_of_day);
    bool client_set_next_scene_level(
        const std::string& level_name,
        const std::string& time_of_day,
        uint8_t reload_count);
    bool reload_required(const LocalSceneLevel& other_level) const;
    LocalSceneLevelLoadStatus load_status() const;
    void notify_level_loaded();
private:
    bool set_next_scene_level(
        const std::string& level_name,
        const std::string& time_of_day,
        uint8_t reload_count);
    mutable SafeAtomicRecursiveSharedMutex mutex_;
    LocalSceneLevelLoadStatus load_status_;
    LocalSceneLevel local_scene_level_;
    LocalSceneLevel next_scene_level_;
    std::function<void()> on_schedule_load_scene_;
    std::function<void()> on_update_time_of_day_;
};

}
