#pragma once
#include <Mlib/Iterator/Guarded_Iterable.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Threads/Safe_Recursive_Shared_Mutex.hpp>
#include <list>
#include <map>
#include <mutex>
#include <string>

namespace Mlib {

class RenderableScene;

enum class RenderableScenesState {
    RUNNING,
    STOPPING,
    SHUTTING_DOWN
};

enum class InsertionStatus {
    SUCCESS,
    FAILURE_NAME_COLLISION,
    FAILURE_SHUTDOWN
};

class RenderableScenes {
    using map_type = std::map<std::string, RenderableScene>;
    RenderableScenes(const RenderableScenes&) = delete;
    RenderableScenes& operator = (const RenderableScenes&) = delete;
public:
    RenderableScenes();
    ~RenderableScenes();
    GuardedIterable<map_type::iterator, std::shared_lock<SafeAtomicRecursiveSharedMutex>> guarded_iterable();
    map_type::iterator unsafe_begin();
    map_type::iterator unsafe_end();
    RenderableScene& operator[](const std::string& name);
    const RenderableScene& operator[](const std::string& name) const;
    RenderableScene* try_get(const std::string& name);
    const RenderableScene* try_get(const std::string& name) const;
    template<class... Args>
    std::pair<map_type::iterator, InsertionStatus> try_emplace(const std::string& k, Args&&... args) {
        // 1. Construct the scene without the "mutex_"
        //    The ctor will lock mutexes in the renderables and should therefore not
        //    lock the "mutex_" variable.
        map_type tmp;
        tmp.try_emplace(k, std::forward<Args>(args)...);
        // auto rs = std::make_unique<RenderableScene>(std::forward<Args>(args)...);
        // 2. Acquire the "mutex_" and append the scene to the list of scenes.
        std::scoped_lock lock{ mutex_ };
        if (state_ != RenderableScenesState::RUNNING) {
            return { renderable_scenes_.end(), InsertionStatus::FAILURE_SHUTDOWN };
        }
        // auto res = renderable_scenes_.try_emplace(k, std::forward<Args>(args)...);
        // auto res = renderable_scenes_.insert({k, std::move(rs)});
        auto res = renderable_scenes_.insert(tmp.extract(k));
        if (res.inserted) {
            renderable_scenes_name_list_.push_back(k);
            return { res.position, InsertionStatus::SUCCESS };
        }
        return { res.position, InsertionStatus::FAILURE_NAME_COLLISION };
    }
    bool shutting_down() const;
private:
    RenderableScenesState state_;
    std::list<std::string> renderable_scenes_name_list_;
    map_type renderable_scenes_;
    mutable SafeAtomicRecursiveSharedMutex mutex_;
};

}
