#include "Scene_Reloader.hpp"
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Remote/Incremental_Objects/Scene_Level.hpp>
#include <Mlib/Remote/Remote_Role.hpp>
#include <Mlib/Threads/Containers/Thread_Safe_String.hpp>
#include <Mlib/Threads/Thread_Safe_Promise.hpp>
#include <mutex>

using namespace Mlib;

SceneReloader::SceneReloader(
    SceneLevelSelector& scene_level_selector,
    ThreadSafeString& next_scene_filename,
    ThreadSafePromise<void>& reload_requested,
    std::optional<RemoteRole> remote_role,
    std::function<std::string()> get_selected_level_id,
    std::function<std::string()> get_selected_time_of_day)
    : scene_level_selector_{scene_level_selector}
    , next_scene_filename_{next_scene_filename}
    , reload_requested_{reload_requested}
    , remote_role_{remote_role}
    , get_selected_level_id_{std::move(get_selected_level_id)}
    , get_selected_time_of_day_{std::move(get_selected_time_of_day)}
{}

SceneReloader::~SceneReloader() = default;

void SceneReloader::load_scene_by_filename(const Utf8Path& filename) {
    std::scoped_lock lock{mutex_};
    linfo() << "Set next_scene_filename (0) = \"" << filename.string() << '"';
    next_scene_filename_ = filename;
    reload_requested_.set();
}

void SceneReloader::set_next_scene_by_manifest(const ReplacementParameterAndFilename& rpe) {
    std::scoped_lock lock{mutex_};
    linfo() << "Set next_scene_filename (1) = \"" << rpe.filename.string() << '"';
    next_scene_filename_ = rpe.filename;
}

void SceneReloader::reload_scene() {
    linfo() << "Reload scene";
    next_scene_filename_ = "";
    reload_requested_.set();
}

void SceneReloader::change_scene() {
    if (remote_role_ == RemoteRole::SERVER) {
        linfo() << "Change scene as server";
        // The scene level selector triggers "local_load_scene_by_filename".
        scene_level_selector_.server_set_next_scene_level(
            get_selected_level_id_(),
            get_selected_time_of_day_());
    } else {
        lerr() << "Change scene as client";
        throw std::runtime_error("Client attempt to change the scene");
    }
}
