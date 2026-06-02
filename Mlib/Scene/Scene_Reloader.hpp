#pragma once
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <functional>
#include <string>

namespace Mlib {

class SceneLevelSelector;
class ThreadSafeString;
struct ReplacementParameterAndFilename;
template <class T>
class ThreadSafePromise;
enum class RemoteRole;

class SceneReloader {
public:
    SceneReloader(
        SceneLevelSelector& scene_level_selector,
        ThreadSafeString& next_scene_filename,
        ThreadSafePromise<void>& reload_requested,
        std::optional<RemoteRole> remote_role,
        std::function<std::string()> get_selected_level_id,
        std::function<std::string()> get_selected_time_of_day);
    ~SceneReloader();
    void load_scene_by_filename(const std::string& filename);
    void set_next_scene_by_manifest(const ReplacementParameterAndFilename& rpe);
    void reload_scene();
    void change_scene();
private:
    SceneLevelSelector& scene_level_selector_;
    ThreadSafeString& next_scene_filename_;
    ThreadSafePromise<void>& reload_requested_;
    std::optional<RemoteRole> remote_role_;
    std::function<std::string()> get_selected_level_id_;
    std::function<std::string()> get_selected_time_of_day_;
    mutable FastMutex mutex_;
};

}
